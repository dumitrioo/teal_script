#pragma once

#include "../commondefs.hpp"
#include "../str_util.hpp"
#include "../terminable.hpp"
#include "../timespec_wrapper.hpp"
#include "../emhash/hash_table8.hpp"
#include "../hash/hash.hpp"
#include "../containers/concurrentqueue.h"
#include "net_utils.hpp"
#include "socket_poller.hpp"
#include "socket_wrapper.hpp"

namespace teal {

    class teal_net_client: public terminable {
    public:
        ~teal_net_client() {
            try {
                disconnect();
            } catch (...) {
            }
        }
        void set_on_conn_established(std::function<void()> fun) {
            std::unique_lock cl{callback_mtp_};
            on_conn_established_ = fun;
        }
        void set_on_conn_closed(std::function<void()> fun) {
            std::unique_lock cl{callback_mtp_};
            on_conn_closed_ = fun;
        }
        void set_on_data_arrived(std::function<void()> fun) {
            std::unique_lock cl{callback_mtp_};
            on_data_arrived_ = fun;
        }

        bool connect(std::string host, std::uint16_t port) {
            if(conn_broken_) {
                disconnect();
            }
            std::unique_lock cl{cnn_mtp_};
            if(connected()) {
                return true;
            } else {
                unterminate();
                conn_broken_ = false;
                if(
                    poller_.create() &&
                    sckt_.create() &&
                    sckt_.connect(host, port) &&
                    sckt_.make_nosigpipe() &&
                    sckt_.make_nonblocking() &&
                    sckt_.make_nodelay() &&
                    sckt_.set_keepalive(true)
                    &&                                                                                                                                         sckt_.ok() &&
                    poller_.add_event(
                        sckt_.handle(),
                        net::POLL_EVENT_IN | net::POLL_EVENT_RDHUP |
                            net::POLL_EVENT_HUP | net::POLL_EVENT_ET
                    )
                ) {
                    thr_ = std::thread{
                        [this]() {
                            while(!termination() && !conn_broken_.load(std::memory_order_acquire)) {
                                try {
                                    std::vector<net::poll_event> evs{};
                                    {
                                        std::unique_lock l{poller_close_mtp_};
                                        evs = poller_.wait(1, timespec_wrapper{0.1});
                                    }
                                    if(!evs.empty()) {
                                        if((evs[0].events & net::POLL_EVENT_IN) == net::POLL_EVENT_IN) {
                                            int bavl{sckt_.bytes_available()};
                                            if(bavl < 0) {
                                                conn_broken_ = true;
                                            } else {
                                                auto dv{net_receive(std::max<int>(bavl, 2 * 1024))};
                                                if(dv) {
                                                    if(!dv->empty()) {
                                                        push_buff_data(std::move(*dv));
                                                        notify_on_data_arrived();
                                                    } else {
                                                        conn_broken_ = true;
                                                    }
                                                } else {
                                                    conn_broken_ = true;
                                                }
                                            }
                                        } else {
                                            conn_broken_ = true;
                                        }
                                    }
                                } catch (...) {
                                    conn_broken_ = true;
                                }
                                if(conn_broken_) {
                                    break;
                                }
                            }
                        }
                    };
                    return true;
                }
                return false;
            }
        }

        bool connected() const {
            return !conn_broken_ && !termination() && sckt_.ok() && thr_.joinable();
        }

        void disconnect() {
            std::unique_lock cl{cnn_mtp_};
            terminate();
            if(thr_.joinable()) { thr_.join(); }
            {
                std::unique_lock l{poller_close_mtp_};
                poller_.close();
            }
            if(sckt_) {
                std::unique_lock lw{write_mtp_};
                std::unique_lock lr{read_mtp_};
                sckt_.close();
            }
            std::unique_lock l{chunks_buffer_mtp_};
            discard_buff_data();
            chunks_buffer_cvar_.notify_all();
        }

        int send(const void *data, size_t data_size) {
            if(connected()) {
                std::unique_lock l{write_mtp_};
                return sckt_.send(data, data_size);
            }
            return 0;
        }

        int send(bytevec const &data) {
            return send(data.data(), data.size());
        }

        int send(std::string const &data) {
            return send(data.data(), data.size());
        }

        std::optional<bytevec> receive(long double timeout = 0) {
            // std::optional<bytevec> opt_data{fetch_buf_data()};
            // if(opt_data) {
            //     return opt_data;
            // }
            // if(timeout > 0) {
            //     std::unique_lock l{chunks_buffer_mtp_};
            //     std::optional<bytevec> opt_data{fetch_buf_data()};
            //     if(opt_data) {
            //         return opt_data;
            //     }
            //     chunks_buffer_cvar_.wait_for(l,
            //         std::chrono::nanoseconds{
            //             static_cast<int64_t>(timeout * 1'000'000'000.0L)
            //         }
            //     );
            // }
            // return fetch_buf_data();

            std::unique_lock l{chunks_buffer_mtp_};
            std::optional<bytevec> opt_data{fetch_buf_data_unlocked()};
            if(opt_data) {
                return opt_data;
            }
            if(timeout > 0) {
                std::cv_status cvs{
                    chunks_buffer_cvar_.wait_for(
                        l,
                        std::chrono::nanoseconds{
                            static_cast<int64_t>(timeout * 1'000'000'000.0L)
                        }
                    )
                };
                if(cvs != std::cv_status::timeout) {
                    return fetch_buf_data_unlocked();
                }
            }
            return {};

        }

    private:
        void notify_on_conn_established() {
            std::shared_lock cl{callback_mtp_};
            if(on_conn_established_) on_conn_established_();
        }

        void notify_on_conn_closed() {
            std::shared_lock cl{callback_mtp_};
            if(on_conn_closed_) on_conn_closed_();
        }

        void notify_on_data_arrived() {
            std::shared_lock cl{callback_mtp_};
            if(on_data_arrived_) on_data_arrived_();
        }

        std::optional<bytevec> net_receive(size_t len) {
            std::unique_lock l{read_mtp_};
            if(!conn_broken_ && !termination() && sckt_.ok()) {
                try {
                    return sckt_.receive(len);
                } catch (std::exception const &e) {
                    std::cerr << e.what();
                } catch (...) {
                    std::cerr << "error in net_receive()";
                }
            }
            return {};
        }

        std::optional<bytevec> fetch_buf_data_unlocked() {
            // std::unique_lock l{chunks_buffer_mtp_};
            bytevec res{};
            bytevec buf{};
            while(!chunks_buffer_.empty()) {
                buf = std::move(chunks_buffer_.front());
                chunks_buffer_.pop_front();
                res.insert(res.end(), buf.begin(), buf.end());
            }
            if(!res.empty()) {
                return res;
            }
            return {};
        }

        void push_buff_data(bytevec const &buf) {
            std::unique_lock l{chunks_buffer_mtp_};
            chunks_buffer_.push_back(buf);
            chunks_buffer_cvar_.notify_all();
        }

        void push_buff_data(bytevec &&buf) {
            std::unique_lock l{chunks_buffer_mtp_};
            chunks_buffer_.push_back(std::move(buf));
            chunks_buffer_cvar_.notify_all();
        }

        bool has_buff_data() const {
            std::unique_lock l{chunks_buffer_mtp_};
            return !chunks_buffer_.empty();
        }

        bool pop_buff_data(bytevec &bv) {
            std::unique_lock l{chunks_buffer_mtp_};
            if(!chunks_buffer_.empty()) {
                bv = chunks_buffer_.front();
                chunks_buffer_.pop_front();
                return true;
            }
            return false;
        }

        void discard_buff_data() {
            std::unique_lock l{chunks_buffer_mtp_};
            chunks_buffer_.clear();
        }

    private:
        mutable std::shared_mutex cnn_mtp_{};
        mutable std::shared_mutex write_mtp_{};
        mutable std::shared_mutex read_mtp_{};

        std::shared_mutex callback_mtp_{};
        std::function<void()> on_conn_established_{nullptr};
        std::function<void()> on_conn_closed_{nullptr};
        std::function<void()> on_data_arrived_{nullptr};

        mutable std::mutex chunks_buffer_mtp_{};
        std::condition_variable chunks_buffer_cvar_{};
        // moodycamel::ConcurrentQueue<bytevec> chunks_buffer_{};
        std::list<bytevec> chunks_buffer_{};

        std::thread thr_{};
        teal::net::socket sckt_{};
        mutable std::mutex poller_close_mtp_{};
        net::socket_poller poller_{};

        std::atomic_bool conn_broken_{false};
    };

}
