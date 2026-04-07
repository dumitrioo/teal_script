#pragma once

#include "../commondefs.hpp"
#include "../str_util.hpp"
#include "../terminable.hpp"
#include "../sequence_generator.hpp"
#include "../timespec_wrapper.hpp"
#include "../emhash/hash_table8.hpp"
#include "../hash/hash.hpp"
#include "../containers/concurrentqueue.h"
#include "net_utils.hpp"
#include "socket_poller.hpp"
#include "socket_wrapper.hpp"

namespace teal {

    using conn_id_t = uint64_t;

    class teal_net_server: public terminable {
    public:
        teal_net_server() = default;
        teal_net_server(teal_net_server const &) = delete;
        teal_net_server &operator=(teal_net_server const &) = delete;
        teal_net_server(teal_net_server &&) = delete;
        teal_net_server &operator=(teal_net_server &&) = delete;
        ~teal_net_server() {
            stop();
        }
        void set_on_data_arrived(std::function<void(conn_id_t)> const &on_data_arrived) {
            std::unique_lock l{on_data_arrived_mtp_};
            on_data_arrived_ = on_data_arrived;
        }

        void set_on_data_arrived(std::function<void(conn_id_t)> &&on_data_arrived) {
            std::unique_lock l{on_data_arrived_mtp_};
            on_data_arrived_ = std::move(on_data_arrived);
        }

        void set_on_new_connection(std::function<void(conn_id_t)> const &on_new_connection) {
            std::unique_lock l{on_new_connection_mtp_};
            on_new_connection_ = on_new_connection;
        }

        void set_on_new_connection(std::function<void(conn_id_t)> &&on_new_connection) {
            std::unique_lock l{on_new_connection_mtp_};
            on_new_connection_ = std::move(on_new_connection);
        }

        void set_on_connection_closed(std::function<void(conn_id_t)> const &on_connection_closed) {
            std::unique_lock l{on_connection_closed_mtp_};
            on_connection_closed_ = on_connection_closed;
        }

        void set_on_connection_closed(std::function<void(conn_id_t)> &&on_connection_closed) {
            std::unique_lock l{on_connection_closed_mtp_};
            on_connection_closed_ = std::move(on_connection_closed);
        }

        void start(const std::string &address, std::uint16_t port, std::size_t num_work_threads) {
            if(conn_broken_) {
                stop();
            }
            std::unique_lock l{mt_mtp_};
            conn_broken_ = false;
            stop_accept_ = false;
            if(!thr_.joinable() && jobs_buffer_workers_.empty()) {
                unterminate();
                poller_.create();
                if(
                    lsk_.create() &&
                    lsk_.make_nonblocking() &&
                    lsk_.bind(address, port) &&
                    lsk_.listen() &&
                    poller_.add_event(lsk_.handle(), net::POLL_EVENT_IN)
                ) {
                    for(size_t i{}; i < num_work_threads; ++i) {
                        jobs_buffer_workers_.emplace_back(
                            [this]() {
                                while(!termination()) {
                                    std::function<void()> fn{};
                                    if(fetch_job(fn)) {
                                        fn();
                                    }
                                }
                            }
                        );
                    }
                    thr_ = std::thread{
                        [this]() {
                            while(!termination()) {
                                try {
                                    std::vector<net::poll_event> events{};
                                    {
                                        // std::unique_lock l{poller_mtp_};
                                        events = poller_.wait(128, timespec_wrapper{0.1});
                                    }
                                    std::size_t evts_size{events.size()};
                                    for(std::size_t curr_evt_indx{0}; curr_evt_indx < evts_size; ++curr_evt_indx) {
                                        if(termination()) {
                                            break;
                                        }
                                        if(lsk_.handle() == events[curr_evt_indx].data.fd) {
                                            if(!stop_accept_.load(std::memory_order_acquire)) {
                                                insert_job([this](){ add_new_conn(); });
                                            }
                                        } else {
                                            auto e{events[curr_evt_indx]};
                                            insert_job([this, e](){ process_poll_event(e); });
                                        }
                                    }
                                } catch (...) {
                                    conn_broken_ = true;
                                }
                                if(conn_broken_) {
                                    std::cout << "tcp server thread exits" << std::endl; ///////////////////////////////////
                                    break;
                                }
                            }
                        }
                    };
                } else {
                    auto es{sys_util::error_str(sys_util::last_error())};
                    if(es.size() > 0) {
                        es[0] = str_util::tolower(es[0]);
                    }
                    throw std::runtime_error{std::string{"error starting server - "} + es};
                }
            }
        }

        void stop() {
            stop_accept_ = true;
            terminate();
            clear_conns();
            {
                std::unique_lock l1{mt_mtp_};
                if(thr_.joinable()) { thr_.join(); }
                for(auto &&t: jobs_buffer_workers_) {
                    if(t.joinable()) { t.join(); }
                }
                jobs_buffer_workers_.clear();
            }
            {
                std::unique_lock l2{jobs_buffer_mtp_};
                jobs_buffer_.clear();
                jobs_buffer_cvar_.notify_all();
            }
            conn_id_gen_.reset(1);
            {
                // std::unique_lock l{poller_mtp_};
                poller_.close();
            }
            lsk_.close();
        }

        bool started() const {
            std::shared_lock l{mt_mtp_};
            return !termination() && !conn_broken_ && !stop_accept_ && lsk_.ok() && thr_.joinable();
        }

        void close_connection(conn_id_t id, bool notify = true) {
            std::shared_ptr<connection> conn{get_conn_by_id(id)};
            if(conn) {
                poller_.del_event(conn->sckt_.handle());
                rm_conn(conn);
                if(notify) {
                    notify_on_connection_closed(conn->conn_id_);
                }
            }
        }

        int send(conn_id_t id, void const *data, std::int32_t data_size) {
            std::shared_ptr<connection> conn{get_conn_by_id(id)};
            if(conn) {
                return conn->send(data, data_size);
            }
            return -1;
        }

        int send(conn_id_t id, bytevec const &data) {
            return send(id, data.data(), data.size());
        }

        int send(conn_id_t id, std::string const &data) {
            return send(id, data.data(), data.size());
        }

        std::optional<bytevec> get_conn_data(conn_id_t id) {
            if(std::shared_ptr<connection> conn{get_conn_by_id(id)}) {
                return conn->pop_buff_data();
            }
            return {};
        }

        std::string get_conn_addr(conn_id_t id) const {
            std::shared_ptr<connection> conn{get_conn_by_id(id)};
            return conn ? conn->get_conn_addr() : std::string{};
        }

    private:
        void add_new_conn() {
            try {
                std::shared_ptr<connection> new_conn{std::make_shared<connection>(this)};
                if(new_conn) {
                    bool accept_failed{false};
                    try {
                        lsk_.accept(new_conn->sckt_);
                    } catch(std::exception const &e) {
                        // std::cerr << e.what() << std::endl;
                        accept_failed = true;
                    } catch(...) {
                        accept_failed = true;
                    }
                    // poller_.re_enable(lsk_.handle(), net::POLL_EVENT_IN | net::POLL_EVENT_ONESHOT);
                    if(accept_failed || !new_conn->sckt_.ok()) {
                        new_conn.reset();
                    }
                    if(new_conn) {
                        if(
                            !new_conn->sckt_.make_nonblocking() || !new_conn->sckt_.make_nodelay() ||
                            !new_conn->sckt_.make_nosigpipe() || !new_conn->sckt_.set_linger(true, 0) ||
                            !new_conn->sckt_.set_keepalive(true) || !new_conn->sckt_.set_tcp_keepidle(90) ||
                            !new_conn->sckt_.set_tcp_keepitvl(30) || !new_conn->sckt_.set_tcp_keepcnt(10)
                        ) {
                            new_conn.reset();
                        }
                    }
                    if(new_conn) {
                        new_conn->conn_id_ = conn_id_gen_();
                        ins_conn(new_conn);
                        bool pol_add_res{false};
                        {
                            // std::unique_lock l{poller_mtp_};
                            pol_add_res = poller_.add_event(new_conn->sckt_.handle(), net::POLL_EVENT_IN | net::POLL_EVENT_ONESHOT);
                        }
                        if(!pol_add_res) {
                            rm_conn(new_conn);
                            new_conn.reset();
                        } else {
                            notify_on_new_connection(new_conn->conn_id_);
                        }
                    }
                }
            } catch(std::exception const &e) {
                // std::cerr << "error: " << e.what() << std::endl;
            } catch(...) {
                // std::cerr << "error accepting socket" << std::endl;
            }
        }

        void process_poll_event(net::poll_event curr_evt) {
            if(termination()) {
                return;
            }
            auto curr_conn{get_conn_by_socket(curr_evt.data.fd)};
            if(curr_conn) {
                bool need_to_rm_sock{false};
                if((curr_evt.events & net::POLL_EVENT_IN) == net::POLL_EVENT_IN) {
                    int bavl{curr_conn->bytes_available()};
                    if(bavl <= 0) {
                        need_to_rm_sock = true;
                    } else {
                        auto dv{curr_conn->receive(std::max<int>(bavl, 4 * 1024))};
                        if(dv) {
                            if(!dv->empty()) {
                                curr_conn->push_buff_data(std::move(*dv));
                                poller_.re_enable(curr_evt.data.fd, net::POLL_EVENT_IN | net::POLL_EVENT_ONESHOT);
                                notify_on_data_arrived(curr_conn->conn_id_);
                            } else {
                                need_to_rm_sock = true;
                            }
                        } else {
                            need_to_rm_sock = true;
                        }
                    }
                } else {
                    need_to_rm_sock = true;
                }
                if(need_to_rm_sock) {
                    rm_conn(curr_conn);
                    notify_on_connection_closed(curr_conn->conn_id_);
                }
            }
        }

        void notify_on_data_arrived(conn_id_t id) {
            std::shared_lock l{on_data_arrived_mtp_};
            if(on_data_arrived_) {
                on_data_arrived_(id);
            }
        }

        void notify_on_new_connection(conn_id_t id) {
            std::shared_lock l{on_new_connection_mtp_};
            if(on_new_connection_) {
                on_new_connection_(id);
            }
        }

        void notify_on_connection_closed(conn_id_t id) {
            std::shared_lock l{on_connection_closed_mtp_};
            if(on_connection_closed_) {
                on_connection_closed_(id);
            }
        }

        class connection {
        public:
            connection(teal_net_server *owner): owner_{owner} {}
            connection(connection const &) = delete;
            connection(connection &&) = delete;
            connection &operator=(connection const &) = delete;
            connection &operator=(connection &&) = delete;
            ~connection() {
                try {
                    close();
                } catch (...) {
                }
            }

            void close() {
                std::unique_lock l{transport_mtp_};
                doomed_ = true;
                {
                    // std::unique_lock l{owner_->poller_mtp_};
                    owner_->poller_.del_event(sckt_.handle());
                }
                sckt_.close();
            }

            int send(const void *data, std::int32_t data_size) {
                int res{-1};
                // std::shared_lock l{transport_mtp_};
                std::unique_lock l{transport_mtp_}; //////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                if(doomed_) {
                    return -1;
                }
                if(data && data_size) {
                    if(sckt_.ok()) {
                        std::unique_lock wl{write_mtp_};
                        res = sckt_.send(data, data_size);
                    } else {
                        return -1;
                    }
                }
                return res;
            }

            int send(bytevec const &data) {
                return send(data.data(), data.size());
            }

            int send(std::string const &data) {
                return send(data.data(), data.size());
            }

            int bytes_available() const {
                return sckt_.bytes_available();
            }

            std::optional<bytevec> receive(int len) {
                std::optional<bytevec> res{};
                if(len) {
                    // std::shared_lock l{transport_mtp_};
                    std::unique_lock l{transport_mtp_}; //////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    if(!doomed_) {
                        if(sckt_.ok()) {
                            try {
                                std::unique_lock rl{read_mtp_};
                                res = sckt_.receive(len);
                            } catch(...) {
                                res = {};
                            }
                        }
                    }
                }
                return res;
            }

            std::string get_conn_addr() const {
                return sckt_.peer_addr();
            }

            void push_buff_data(bytevec const &buf) {
                std::unique_lock l{chunks_buffer_mtp_};
                chunks_buffer_.enqueue(buf);
            }

            void push_buff_data(bytevec &&buf) {
                std::unique_lock l{chunks_buffer_mtp_};
                chunks_buffer_.enqueue(std::move(buf));
            }

            bool has_buff_data() const {
                std::shared_lock l{chunks_buffer_mtp_};
                return chunks_buffer_.size_approx() > 0;
            }

            std::optional<bytevec> pop_buff_data() {
                bytevec b;
                bytevec bv{};
                std::unique_lock l{chunks_buffer_mtp_};
                while(chunks_buffer_.try_dequeue(b)) {
                    bv.insert(bv.end(), b.begin(), b.end());
                }
                if(!bv.empty()) {
                    return bv;
                }
                return {};
            }

            teal_net_server *owner_{nullptr};
            mutable std::shared_mutex chunks_buffer_mtp_{};
            moodycamel::ConcurrentQueue<bytevec> chunks_buffer_{};
            conn_id_t conn_id_{0};
            std::shared_mutex transport_mtp_{};
            std::shared_mutex write_mtp_{};
            std::shared_mutex read_mtp_{};
            net::socket sckt_{};
            bool doomed_{false};
        };

        bool ins_conn(std::shared_ptr<connection> conn) {
            if(conn && conn->sckt_.ok()) {
                std::unique_lock l{connections_mtp_};
                if(skfd_to_conn_.find(conn->sckt_.handle()) == skfd_to_conn_.end() &&
                    id_to_conn_.find(conn->conn_id_) == id_to_conn_.end()
                ) {
                    skfd_to_conn_[conn->sckt_.handle()] = conn;
                    id_to_conn_[conn->conn_id_] = conn;
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }

        void rm_conn(std::shared_ptr<connection> conn) {
            std::unique_lock l{connections_mtp_};
            auto id_it{id_to_conn_.find(conn->conn_id_)};
            auto sk_it{skfd_to_conn_.find(conn->sckt_.handle())};
            if(id_it != id_to_conn_.end()) { id_to_conn_.erase(id_it); }
            if(sk_it != skfd_to_conn_.end()) { skfd_to_conn_.erase(sk_it); }
        }

        void clear_conns() {
            std::unique_lock l{connections_mtp_};
            id_to_conn_.clear();
            skfd_to_conn_.clear();
        }

        std::shared_ptr<connection> get_conn_by_socket(int skfd) const {
            std::shared_lock l{connections_mtp_};
            auto it{skfd_to_conn_.find(skfd)};
            return it == skfd_to_conn_.end() ? std::shared_ptr<connection>{} : it->second;
        }

        std::shared_ptr<connection> get_any_conn() const {
            std::shared_lock l{connections_mtp_};
            if(!skfd_to_conn_.empty()) {
                return skfd_to_conn_.begin()->second;
            }
            return std::shared_ptr<connection>{};
        }

        std::size_t num_conns() const {
            std::shared_lock l{connections_mtp_};
            return skfd_to_conn_.size();
        }

        std::shared_ptr<connection> get_conn_by_id(conn_id_t cnnid) const {
            std::shared_lock l{connections_mtp_};
            auto it{id_to_conn_.find(cnnid)};
            return it == id_to_conn_.end() ? std::shared_ptr<connection>{} : it->second;
        }

    private:
        bool fetch_job(std::function<void()> &fn) {
            std::unique_lock l{jobs_buffer_mtp_};
            if(!jobs_buffer_.empty()) {
                fn = std::move(jobs_buffer_.front());
                jobs_buffer_.pop_front();
                return true;
            }
            std::cv_status waitstatus{jobs_buffer_cvar_.wait_for(l, std::chrono::milliseconds{100})};
            if(waitstatus == std::cv_status::no_timeout) {
                if(!jobs_buffer_.empty()) {
                    fn = std::move(jobs_buffer_.front());
                    jobs_buffer_.pop_front();
                    return true;
                }
            }
            return false;
        }

        bool insert_job(std::function<void()> &&fn) {
            if(termination()) { return false; }
            std::unique_lock l{jobs_buffer_mtp_};
            jobs_buffer_.push_back(std::move(fn));
            jobs_buffer_cvar_.notify_all();
            return true;
        }

        bool insert_job(std::function<void()> const &fn) {
            if(termination()) { return false; }
            std::unique_lock l{jobs_buffer_mtp_};
            jobs_buffer_.push_back(fn);
            jobs_buffer_cvar_.notify_all();
            return true;
        }

    private:
        atomic_sequence_generator<conn_id_t> conn_id_gen_{1};

        mutable std::shared_mutex mt_mtp_{};
        std::thread thr_{};
        net::socket lsk_{};
        // mutable std::mutex poller_mtp_{};
        net::socket_poller poller_{};
        mutable std::mutex jobs_buffer_mtp_{};
        std::condition_variable jobs_buffer_cvar_{};
        std::list<std::thread> jobs_buffer_workers_{};
        //moodycamel::ConcurrentQueue<std::function<void()>> jobs_buffer_{};
        std::list<std::function<void()>> jobs_buffer_{};

        mutable std::shared_mutex connections_mtp_{};
        emhash8::HashMap<int, std::shared_ptr<connection>, num_cast_hash<int>> skfd_to_conn_{};
        emhash8::HashMap<conn_id_t, std::shared_ptr<connection>, num_cast_hash<conn_id_t>> id_to_conn_{};

        mutable std::shared_mutex on_data_arrived_mtp_{};
        std::function<void(conn_id_t)> on_data_arrived_{nullptr};
        mutable std::shared_mutex on_new_connection_mtp_{};
        std::function<void(conn_id_t)> on_new_connection_{nullptr};
        mutable std::shared_mutex on_connection_closed_mtp_{};
        std::function<void(conn_id_t)> on_connection_closed_{nullptr};

        std::atomic_bool stop_accept_{false};
        std::atomic_bool conn_broken_{false};
    };

}
