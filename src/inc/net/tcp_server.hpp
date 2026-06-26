#pragma once

#include "../commondefs.hpp"
#include "../str_util.hpp"
#include "../sys_util.hpp"
#include "../terminable.hpp"
#include "../sequence_generator.hpp"
#include "../timespec_wrapper.hpp"
#include "../emhash/hash_table8.hpp"
#include "../hash/hash.hpp"
#include "../containers/concurrentqueue.h"
#include "net_utils.hpp"
#include "socket_poller.hpp"
#include "socket_wrapper.hpp"
#ifdef TEAL_TCPSERVER_USE_SSL
#include "ssl/ssl_wrappers.hpp"
#include "ssl/ssl_utils.hpp"
#endif

namespace teal::net {

    using conn_id_t = uint64_t;

    template<typename QUEUE_T>
    class tcp_server: public terminable {
    public:
        using conn_id_t = std::uint64_t;
        using size_type = std::uint64_t;
        using ssize_type = std::int64_t;

        tcp_server(bool use_nodelay_writes = false):
            use_nodelay_writes_{use_nodelay_writes}
        {
        }

        tcp_server(
            bool use_nodelay_writes,
            QUEUE_T *cq,
            std::string const &bind_address,
            int port,
            int wait_events_count = 256,
            bool enable_ssl = false,
            std::string priv_file_name = {},
            std::string pub_file_name = {}
        ):
            use_nodelay_writes_{use_nodelay_writes}
        {
            start(cq, bind_address, port, wait_events_count, enable_ssl, priv_file_name, pub_file_name);
        }

        tcp_server(
            bool use_nodelay_writes,
            QUEUE_T *cq,
            std::string const &bind_address, int port,
            int wait_events_count = 256, bool enable_ssl = false,
            bytevec priv = {},
            bytevec pub = {}
        ):
            use_nodelay_writes_{use_nodelay_writes}
        {
            start(cq, bind_address, port, wait_events_count, enable_ssl, priv, pub);
        }

        ~tcp_server() {
            terminate();
            stop();
        }

        void start(
            QUEUE_T *cq,
            std::string const &bind_address, int port, int wait_events_count,
            bool enable_ssl, std::string priv_file_name, std::string pub_file_name
        ) {
            std::unique_lock l{started_mtp_};
            if(!started_) {
                if(!cq) {
                    throw std::runtime_error{"valid command queue is mandatory argument"};
                }
                unterminate();
                cq_ = cq;
                wait_events_count_ = wait_events_count;
                set_ssl_enabled(enable_ssl);
                set_keys(priv_file_name, pub_file_name);
                lsnr_ = std::make_unique<teal::net::socket>();
                if(!lsnr_->create()) { throw_errno(errno); }
                if(!lsnr_->set_reuse_addr(true)) { throw_errno(errno); }
                if(!lsnr_->bind(bind_address, port)) { throw_errno(errno); }
                if(!lsnr_->listen()) { throw_errno(errno); }
                if(!lsnr_->make_nonblocking()) { throw_errno(errno); }
                poller_ = std::make_unique<teal::net::socket_poller>();
                if(!poller_->add_event(lsnr_->handle(), teal::net::POLL_EVENT_IN | teal::net::POLL_EVENT_ET)) {
                    throw_errno(errno);
                }
                cq_->enqueue(fn_);
                started_ = true;
            }
        }

        void start(
            QUEUE_T *cq,
            std::string const &bind_address, int port, int wait_events_count,
            bool enable_ssl, teal::bytevec priv, teal::bytevec pub
            ) {
            std::unique_lock l{started_mtp_};
            if(!started_) {
                if(!cq) {
                    throw std::runtime_error{"valid command queue is mandatory argument"};
                }
                unterminate();
                cq_ = cq;
                wait_events_count_ = wait_events_count;
                set_ssl_enabled(enable_ssl);
                set_keys(priv, pub);
                lsnr_ = std::make_unique<teal::net::socket>();
                if(!lsnr_->create()) { throw_errno(errno); }
                if(!lsnr_->set_reuse_addr(true)) { throw_errno(errno); }
                if(!lsnr_->bind(bind_address, port)) { throw_errno(errno); }
                if(!lsnr_->listen()) { throw_errno(errno); }
                if(!lsnr_->make_nonblocking()) { throw_errno(errno); }
                poller_ = std::make_unique<teal::net::socket_poller>();
                if(!poller_->add_event(lsnr_->handle(), teal::net::POLL_EVENT_IN | teal::net::POLL_EVENT_ET)) {
                    throw_errno(errno);
                }
                cq_->enqueue(fn_);
                started_ = true;
            }
        }

        void stop() {
            std::unique_lock l1{started_mtp_};
            if(started_) {
                terminate();
                while(num_conns() > 0) {
                    std::shared_ptr<connection> c{get_any_conn()};
                    if(c) {
                        {
                            std::shared_lock l{poller_mtp_};
                            if(poller_) poller_->del_event(c->sckt_->handle());
                        }
                        c->close();
                        rm_conn(c);
                        cq_->enqueue([this, c]() {
                            notify_on_connection_closed(c->conn_id_);
                        });
                    }
                }
                {
                    std::unique_lock l{lsnr_mtp_};
                    std::unique_lock p{poller_mtp_};
                    poller_->del_event(lsnr_->handle());
                    lsnr_.reset();
                    poller_.reset();
                }
                set_ssl_enabled(false);
                started_ = false;

                while(uses_ != 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        }

        bool started() const {
            std::shared_lock l{started_mtp_};
            return started_;
        }

        void set_ssl_enabled(bool
#ifdef TEAL_TCPSERVER_USE_SSL
                val
#endif
        ) {
#ifdef TEAL_TCPSERVER_USE_SSL
            std::unique_lock l{ssl_mtp_};
            if(val) {
                ssl_ctx_ = std::make_unique<teal::ssl::context>(true);
                ssl_ctx_->init();
            } else {
                ssl_ctx_.reset();
            }
#endif
        }

        bool set_keys(std::string
#ifdef TEAL_TCPSERVER_USE_SSL
                          priv_file_name
#endif
                      , std::string
#ifdef TEAL_TCPSERVER_USE_SSL
                          pub_file_name
#endif
        ) {
#ifdef TEAL_TCPSERVER_USE_SSL
            std::unique_lock l{ssl_mtp_};
            try {
                if(ssl_ctx_ && teal::file_util::file_exists(priv_file_name) && teal::file_util::file_exists(pub_file_name)) {
                    ssl_ctx_->set_server_keys(priv_file_name, pub_file_name);
                    return true;
                }
            } catch(...) {
            }
#endif
            return false;
        }

        bool set_keys(teal::bytevec
#ifdef TEAL_TCPSERVER_USE_SSL
                          priv
#endif
                      , teal::bytevec
#ifdef TEAL_TCPSERVER_USE_SSL
                          pub
#endif
        ) {
#ifdef TEAL_TCPSERVER_USE_SSL
            std::unique_lock l{ssl_mtp_};
            try {
                if(ssl_ctx_) {
                    ssl_ctx_->set_server_keys(priv, pub);
                    return true;
                }
            } catch(...) {
            }
#endif
            return false;
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

        void close_connection(conn_id_t id, bool notify = true) {
            std::shared_ptr<connection> conn{get_conn_by_id(id)};
            if(conn) {
                {
                    std::shared_lock l{poller_mtp_};
                    poller_->del_event(conn->sckt_->handle());
                }
                rm_conn(conn);
                if(notify) {
                    notify_on_connection_closed(conn->conn_id_);
                }
            }
        }

        int send(conn_id_t id, void const *data, std::int32_t data_size, long double timeout_seconds = 15) {
            std::shared_ptr<connection> conn{get_conn_by_id(id)};
            if(conn) {
                return conn->send(data, data_size, timeout_seconds);
            }
            return -1;
        }

        int send(conn_id_t id, teal::bytevec const &data, long double timeout_seconds = 15) {
            return send(id, data.data(), data.size(), timeout_seconds);
        }

        std::optional<teal::bytevec> get_conn_data(conn_id_t id) {
            std::shared_ptr<connection> conn{get_conn_by_id(id)};
            teal::bytevec bv{};
            if(conn && conn->pop_buff_data(bv)) {
                return bv;
            }
            return {};
        }

        void send_async(conn_id_t id, void const *data, std::int32_t data_size, std::function<void(int)> send_callback = nullptr, long double timeout_seconds = 15) {
            cq_->enqueue([&]() {
                uses_++;
                teal::shut_on_destroy at_exit{[&]() { uses_--; }};
                std::shared_ptr<connection> conn{get_conn_by_id(id)};
                if(conn) {
                    int res{conn->send(data, data_size, timeout_seconds)};
                    if(send_callback) {
                        send_callback(res);
                    }
                } else {
                    if(send_callback) {
                        send_callback(-1);
                    }
                }
            });
        }

        std::string get_conn_addr(conn_id_t id) const {
            std::shared_ptr<connection> conn{get_conn_by_id(id)};
            return conn ? conn->get_conn_addr() : std::string{};
        }

    private:
        std::function<void()> fn_{
            [&]() {
                uses_++;
                teal::shut_on_destroy at_exit{[&]() { uses_--; }};
                if(!termination()) {
                    try {
                        std::vector<teal::net::poll_event> events{};
                        {
                            std::shared_lock l{poller_mtp_};
                            if(poller_.get() != nullptr) {
                                events = poller_->wait(wait_events_count_, teal::timespec_wrapper{0.25});
                            } else {
                                l.unlock();
                                if(!termination()) { std::this_thread::sleep_for(std::chrono::milliseconds{50}); }
                            }
                        }
                        if(!termination()) {
                            std::size_t evts_size{events.size()};
                            for(std::size_t curr_evt_indx{0}; curr_evt_indx < evts_size; ++curr_evt_indx) {
                                if(termination()) {
                                    break;
                                }
                                if(lsnr_->handle() == events[curr_evt_indx].data.fd) {
                                    add_new_conn();
                                } else {
                                    process_poll_event(events[curr_evt_indx]);
                                }
                            }
                        }
                    } catch(...) {
                    }
                    if(!termination()) {
                        cq_->enqueue(fn_);
                    }
                }
            }
        };

        class connection {
        public:
            connection() = default;
            connection(connection const &) = delete;
            connection(connection &&) = delete;
            connection &operator=(connection const &) = delete;
            connection &operator=(connection &&) = delete;
            ~connection() {
                try {
                    close();
                } catch(...) {
                }
            }

            void close() {
                {
                    std::unique_lock l{transport_mtp_};
                    doomed_ = true;
#ifdef TEAL_TCPSERVER_USE_SSL
                    if(ssl_conn_) {
                        ssl_conn_->close();
                    }
#endif
                    sckt_->close();
                }

                std::unique_lock l1{chunks_buffer_mtp_};
                chunks_buffer_.clear();
            }

            std::int64_t send(const void *data, std::int64_t data_size, long double timeout_seconds) {
                std::int64_t res{-1};
                std::shared_lock l{transport_mtp_};
                if(doomed_) {
                    return -1;
                }
                std::int64_t off = 0;
                if(data && data_size) {
                    std::unique_lock wl{write_mtp_};
                    long double last_sent_moment{steady_time_sec()};
                    while(off < data_size) {
                        std::int64_t to_send{(std::min<std::int64_t>)(32768, data_size - off)};
                        if(sckt_ && sckt_->ok()) {
                            try {
                                std::int64_t wr{-1};
#ifdef TEAL_TCPSERVER_USE_SSL
                                if(ssl_conn_) {
                                    wr = ssl_conn_->send_data((int8_t const *)data + off, to_send);
                                    last_sent_moment = steady_time_sec();
                                } else
#endif
                                {
                                    wr = sckt_->send((int8_t const *)data + off, to_send);
                                    last_sent_moment = steady_time_sec();
                                }
                                if(wr > 0) {
                                    off += wr;
                                }
                            } catch(errno_eagain const &e) {
                                if(steady_time_sec() - last_sent_moment > timeout_seconds) {
                                    return -1;
                                }
                                std::this_thread::sleep_for(std::chrono::microseconds{100});
                            } catch(errno_ewouldblock const &e) {
                                if(steady_time_sec() - last_sent_moment > timeout_seconds) {
                                    return -1;
                                }
                                std::this_thread::sleep_for(std::chrono::microseconds{100});
                            } catch(...) {
                                return -1;
                            }
                        } else {
                            return -1;
                        }
                    }
                    res = off;
                }
                return res;
            }

            teal::bytevec receive(int len) {
                std::shared_lock l{transport_mtp_};
                if(!doomed_) {
                    if(sckt_ && sckt_->ok()) {
                        std::unique_lock rl{read_mtp_};
#ifdef TEAL_TCPSERVER_USE_SSL
                        if(ssl_conn_) {
                            return ssl_conn_->receive(len);
                        } else
#endif
                        {
                            return sckt_->receive(len);
                        }
                    }
                }
                return {};
            }

            void push_buff_data(teal::bytevec const &buf) {
                std::unique_lock l{chunks_buffer_mtp_};
                chunks_buffer_.push_back(buf);
            }

            void push_buff_data(teal::bytevec &&buf) {
                std::unique_lock l{chunks_buffer_mtp_};
                chunks_buffer_.push_back(std::move(buf));
            }

            bool has_buff_data() const {
                std::shared_lock l{chunks_buffer_mtp_};
                return !chunks_buffer_.empty();
            }

            bool pop_buff_data(teal::bytevec &bv) {
                std::unique_lock l{chunks_buffer_mtp_};
                size_t total{0};
                while(!chunks_buffer_.empty()) {
                    if(!chunks_buffer_.front().empty()) {
                        total += chunks_buffer_.front().size();
                        bv.insert(bv.end(), chunks_buffer_.front().begin(), chunks_buffer_.front().end());
                    }
                    chunks_buffer_.pop_front();
                }
                return total > 0;
            }

            std::string get_conn_addr() const {
                return sckt_->peer_addr();
            }

            conn_id_t conn_id_{0};
            std::shared_mutex transport_mtp_{};
            std::shared_mutex write_mtp_{};
            std::shared_mutex read_mtp_{};
            std::shared_ptr<teal::net::socket> sckt_{std::make_shared<teal::net::socket>()};
#ifdef TEAL_TCPSERVER_USE_SSL
            std::shared_ptr<teal::ssl::connection> ssl_conn_{nullptr};
#endif
            mutable std::shared_mutex chunks_buffer_mtp_{};
            std::list<teal::bytevec> chunks_buffer_{};
            bool doomed_{false};
        };

        void add_new_conn() {
            cq_->enqueue([&]() {
                uses_++;
                teal::shut_on_destroy at_exit{[&]() { uses_--; }};
                try {
                    while(true) {
                        std::shared_ptr<connection> new_conn{std::make_shared<connection>()};
                        try {
                            lsnr_->accept(*(new_conn->sckt_));
                            new_conn->sckt_->make_nosigpipe();
                        } catch(...) {
                            break;
                        }
                        bool err{false};
                        if(use_nodelay_writes_) {
                            err = !new_conn->sckt_->make_nodelay();
                        }
                        if(
                            !err
                            &&
                            new_conn->sckt_->make_nonblocking()
                        ) {
#ifdef TEAL_TCPSERVER_USE_SSL
                            std::shared_lock l{ssl_mtp_};
                            if(ssl_ctx_) {
                                new_conn->ssl_conn_ = ssl_ctx_->get_new_connection(new_conn->sckt_, {});
                                if(!new_conn->ssl_conn_) {
                                    new_conn.reset();
                                }
                            }
#endif
                        } else {
                            new_conn.reset();
                        }
                        if(new_conn) {
                            new_conn->conn_id_ = conn_id_gen_();
                            ins_conn(new_conn);
                            std::shared_lock l{poller_mtp_};
                            if(!poller_->add_event(new_conn->sckt_->handle(), teal::net::POLL_EVENT_IN | teal::net::POLL_EVENT_ET)) {
                                l.unlock();
                                rm_conn(new_conn);
                                new_conn.reset();
                            } else {
                                l.unlock();
                                notify_on_new_connection(new_conn->conn_id_);
                            }
                        }
                    }
                } catch(std::exception const &e) {
                    std::cerr << "error: " << e.what() << std::endl;
                } catch(...) {
                    std::cerr << "error accepting socket" << std::endl;
                }
            });
        }

        void process_poll_event(teal::net::poll_event curr_evt) {
            if(termination()) {
                return;
            }
            uses_++;
            cq_->enqueue([this, curr_evt]() {
                teal::shut_on_destroy at_exit{[&]() { uses_--; }};
                auto curr_conn{get_conn_by_socket(curr_evt.data.fd)};
                if(curr_conn) {
                    bool need_to_rm_sock{false};
                    if(curr_evt.events & teal::net::POLL_EVENT_IN) {
                        bool ok{true};
                        size_t numtotal{0};
                        while(true) {
                            try {
                                auto dv{curr_conn->receive(32 * 1024)};
                                if(dv.size() > 0) {
                                    numtotal += dv.size();
                                    curr_conn->push_buff_data(std::move(dv));
                                } else {
                                    ok = false;
                                    need_to_rm_sock = true;
                                    break;
                                }
                            } catch(errno_eagain const &ea) {
                                break;
                            } catch(errno_ewouldblock const &ea) {
                                break;
                            } catch(...) {
                                ok = false;
                                need_to_rm_sock = true;
                                break;
                            }
                        }
                        if(numtotal > 0 && ok && !need_to_rm_sock) {
                            notify_on_data_arrived(curr_conn->conn_id_);
                        }
                    } else {
                        need_to_rm_sock = true;
                    }
                    if(need_to_rm_sock) {
                        {
                            std::shared_lock l{poller_mtp_};
                            poller_->del_event(curr_evt.data.fd);
                        }
                        rm_conn(curr_conn);
                        notify_on_connection_closed(curr_conn->conn_id_);
                    }
                }
            });
        }

        bool ins_conn(std::shared_ptr<connection> conn) {
            if(conn && conn->sckt_->ok()) {
                std::unique_lock l{connections_mtp_};
                if(skfd_to_conn_.find(conn->sckt_->handle()) == skfd_to_conn_.end() &&
                    id_to_conn_.find(conn->conn_id_) == id_to_conn_.end()
                    ) {
                    skfd_to_conn_[conn->sckt_->handle()] = conn;
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
            auto sk_it{skfd_to_conn_.find(conn->sckt_->handle())};
            if(id_it != id_to_conn_.end()) { id_to_conn_.erase(id_it); }
            if(sk_it != skfd_to_conn_.end()) { skfd_to_conn_.erase(sk_it); }
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

        std::shared_mutex on_data_arrived_mtp_{};
        std::function<void(conn_id_t)> on_data_arrived_{nullptr};
        std::shared_mutex on_new_connection_mtp_{};
        std::function<void(conn_id_t)> on_new_connection_{nullptr};
        std::shared_mutex on_connection_closed_mtp_{};
        std::function<void(conn_id_t)> on_connection_closed_{nullptr};

        std::atomic<size_type> uses_{0};
        QUEUE_T *cq_{};
        teal::atomic_sequence_generator<conn_id_t> conn_id_gen_{1};

        mutable std::shared_mutex connections_mtp_{};
        std::map<int, std::shared_ptr<connection>> skfd_to_conn_{};
        std::map<conn_id_t, std::shared_ptr<connection>> id_to_conn_{};

        std::shared_mutex poller_mtp_{};
        std::unique_ptr<teal::net::socket_poller> poller_{};
        std::shared_mutex lsnr_mtp_{};
        std::unique_ptr<teal::net::socket> lsnr_{};
        std::shared_mutex ssl_mtp_{};
#ifdef TEAL_TCPSERVER_USE_SSL
        std::unique_ptr<teal::ssl::context> ssl_ctx_{nullptr};
#endif
        int wait_events_count_{1024};
        mutable std::shared_mutex started_mtp_{};
        bool started_{false};
        bool use_nodelay_writes_{false};
    };

}
