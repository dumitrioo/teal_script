#pragma once

#include "../commondefs.hpp"
#include "../timespec_wrapper.hpp"
#include "../serialization.hpp"
#include "../terminable.hpp"
#include "../crypto/gamma.hpp"
#include "../sequence_generator.hpp"
#include "../emhash/hash_table8.hpp"
#include "net_utils.hpp"
#include "socket_wrapper.hpp"
#include "net_data_transfer.hpp"
#include "socket_poller.hpp"
#include "udp_hlp.hpp"

namespace teal::net {

    template<std::size_t NET_PACKET_PAYLOAD_SIZE_MAX = 1450>
    class udp_server_muxed: public teal::terminable {
        class udp_connection {
        public:
            udp_connection(
                struct sockaddr_in cliaddr4,
                struct sockaddr_in6 cliaddr6,
                address_family af,
                std::uint64_t conn_id
            ):
                conn_id_{conn_id}
            {
                if(af == address_family::inet4) {
                    cli_addr_.v4_ = cliaddr4;
                } else if(af == address_family::inet6) {
                    cli_addr_.v6_ = cliaddr6;
                }
            }
            udp_connection(udp_connection const &) = delete;
            udp_connection(udp_connection &&) = delete;
            udp_connection &operator=(udp_connection const &) = delete;
            udp_connection &operator=(udp_connection &&) = delete;
            ~udp_connection() = default;

            void update_last_activity_time() {
                std::unique_lock l{last_activity_time_mtp_};
                last_activity_time_ = teal::curr_timestamp_seconds();
            }

            long double seconds_since_last_activity() const {
                std::shared_lock l{last_activity_time_mtp_};
                return teal::curr_timestamp_seconds() - last_activity_time_;
            }

            std::uint64_t conn_id() const {
                return conn_id_;
            }

            struct sockaddr const *addr_ptr() const {
                return (struct sockaddr const *)&cli_addr_;
            }

            void set_ack() {
                ack_ = true;
            }

            bool ack() const {
                return ack_;
            }

            std::optional<teal::bytevec> set_incoming_data(void const *d, std::uint64_t ds) {
                update_last_activity_time();
                std::unique_lock l{demuxer_mtp_};
                remove_stale_inputs_unlocked(120);
                return demuxer_.add_data(d, ds);
            }

            void set_output_message(std::vector<std::uint8_t> const &msg) {
                std::unique_lock l{muxer_mtp_};
                return muxer_.add_message(msg);
            }

            std::optional<bytevec> fetch_out_chunk() {
                std::unique_lock l{muxer_mtp_};
                return muxer_.fetch_out_chunk();
            }

            std::vector<std::uint8_t> encrypt_data(void const *p, std::size_t ps, std::uint64_t ctr_start) {
                if(!p || !ps) { return {}; }
                std::unique_lock lck{encrypt_mtp_};
                teal::crypt::gamma out_gamma{out_gamma_};
                lck.unlock();
                std::vector<std::uint8_t> e{(std::uint8_t const *)p, (std::uint8_t const *)p + ps};
                std::uint64_t crypto_ctr{ctr_start};
                for(std::size_t i{0}; i < e.size(); ++i) {
                    e[i] = e[i] ^ out_gamma[crypto_ctr++];
                }
                return e;
            }

            std::vector<std::uint8_t> decrypt_data(void const *e, std::size_t es, std::uint64_t ctr_start) {
                if(!e || !es) { return {}; }
                std::vector<std::uint8_t> res{(std::uint8_t const *)e, (std::uint8_t const *)e + es};
                std::uint64_t crypto_ctr{ctr_start};
                std::unique_lock lck{decrypt_mtp_};
                teal::crypt::gamma in_gamma{in_gamma_};
                lck.unlock();
                for(std::size_t i{0}; i < res.size(); ++i) {
                    res[i] = res[i] ^ in_gamma[crypto_ctr++];
                }
                return res;
            }

            void set_local_key(teal::bytevec const &key, teal::bytevec const &iv) {
                crypto_ctr_ = 0;
                out_gamma_.init(key.data(), key.size(), iv.data(), iv.size());
            }

            void set_remote_key(teal::bytevec const &key, teal::bytevec const &iv) {
                in_gamma_.init(key.data(), key.size(), iv.data(), iv.size());
            }

            void clear_local_key() {
                out_gamma_.clear();
            }

            void clear_remote_key() {
                in_gamma_.clear();
            }

            void set_encryption_enabled(bool val) {
                encryption_on_ = val;
            }

            bool encryption_enabled() const {
                return encryption_on_;
            }

            std::uint64_t crypto_ctr_for_data_size(std::uint64_t ds) {
                return crypto_ctr_.fetch_add(ds);
            }

            template<typename T>
            void set_user_data(T const &val) {
                user_data_ = val;
            }

            template<typename T>
            T const &user_data() const {
                return std::any_cast<T const &>(user_data_);
            }

        private:
            std::any user_data_{};

            std::mutex muxer_mtp_{};
            teal::net::packets_muxer<std::uint16_t> muxer_{NET_PACKET_PAYLOAD_SIZE_MAX};
            std::mutex demuxer_mtp_{};
            teal::net::packets_demuxer demuxer_{};

            void remove_stale_inputs_unlocked(long double seconds_old) {
                if(demuxer_.queued_items() > 10) {
                    demuxer_.remove_queued_items_older_than_seconds(seconds_old);
                }
            }

        private:
            union {
                struct sockaddr_in v4_;
                struct sockaddr_in6 v6_;
            } cli_addr_;
            std::uint64_t conn_id_{0};

            mutable std::shared_mutex last_activity_time_mtp_{};
            long double last_activity_time_{teal::curr_timestamp_seconds()};

            bool ack_{false};

            std::mutex encrypt_mtp_{};
            std::mutex decrypt_mtp_{};
            std::atomic<std::uint64_t> crypto_ctr_{0};
            teal::crypt::gamma out_gamma_{};
            teal::crypt::gamma in_gamma_{};
            std::atomic<bool> encryption_on_{false};
        };

    public:
        udp_server_muxed(
            // teal::command_queue *cq,
            std::size_t num_work_threads,
            long double recv_timeout = 0.1L,
            bool async = false,
            address_family af = address_family::inet4
        ):
            // cq_{cq},
            recv_timeout_{recv_timeout},
            sock_type_{af},
            async_{async}
        {
            std::unique_lock l2{jobs_buffer_workers_mtp_};
            for(size_t i{}; i < num_work_threads; ++i) {
                jobs_buffer_workers_.emplace_back(
                    [this]() {
                        while(!stop_jobs_) {
                            std::function<void()> fn{};
                            if(fetch_job(fn)) {
                                fn();
                            }
                        }
                    }
                );
            }
        }

        udp_server_muxed(udp_server_muxed const &) = delete;

        udp_server_muxed(udp_server_muxed &&) = delete;

        udp_server_muxed &operator=(udp_server_muxed const &) = delete;

        udp_server_muxed &operator=(udp_server_muxed &&) = delete;

        ~udp_server_muxed() {
            stop();
            stop_jobs_ = true;
            {
                std::unique_lock l2{jobs_buffer_workers_mtp_};
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
        }

        void set_on_data_from_client(std::function<void(std::uint64_t, void const *, std::size_t)> const &on_data_from_client) {
            on_data_from_client_ = on_data_from_client;
        }

        void set_on_data_arrived(std::function<void(std::uint64_t, void const *, std::size_t)> &&on_data_from_client) {
            on_data_from_client_ = std::move(on_data_from_client);
        }

        void set_on_new_connection(std::function<void(std::uint64_t)> const &on_new_connection) {
            on_new_connection_ = on_new_connection;
        }

        void set_on_new_connection(std::function<void(std::uint64_t)> &&on_new_connection) {
            on_new_connection_ = std::move(on_new_connection);
        }

        void set_on_connection_closed(std::function<void(std::uint64_t)> const &on_connection_closed) {
            on_connection_closed_ = on_connection_closed;
        }

        void set_on_connection_closed(std::function<void(std::uint64_t)> &&on_connection_closed) {
            on_connection_closed_ = std::move(on_connection_closed);
        }

        void set_connection_local_key(std::uint64_t conn_id, teal::bytevec const &key, teal::bytevec const &iv) {
            std::shared_ptr<udp_connection> conn_ptr{connection_by_id(conn_id)};
            if(conn_ptr) {
                conn_ptr->set_local_key(key, iv);
            }
        }

        void set_connection_remote_key(std::uint64_t conn_id, teal::bytevec const &key, teal::bytevec const &iv) {
            std::shared_ptr<udp_connection> conn_ptr{connection_by_id(conn_id)};
            if(conn_ptr) {
                conn_ptr->set_remote_key(key, iv);
            }
        }

        void clear_connection_local_key(std::uint64_t conn_id) {
            std::shared_ptr<udp_connection> conn_ptr{connection_by_id(conn_id)};
            if(conn_ptr) {
                conn_ptr->clear_local_key();
            }
        }

        void clear_connection_remote_key(std::uint64_t conn_id) {
            std::shared_ptr<udp_connection> conn_ptr{connection_by_id(conn_id)};
            if(conn_ptr) {
                conn_ptr->clear_remote_key();
            }
        }

        void set_connection_encryption_enabled(std::uint64_t conn_id, bool val) {
            std::shared_ptr<udp_connection> conn_ptr{connection_by_id(conn_id)};
            if(conn_ptr) {
                conn_ptr->set_encryption_enabled(val);
            }
        }

        bool connection_encryption_enabled(std::uint64_t conn_id) const {
            std::shared_ptr<udp_connection> conn_ptr{connection_by_id(conn_id)};
            if(conn_ptr) {
                return conn_ptr->encryption_enabled();
            }
            return false;
        }

        template<typename T>
        void set_conn_user_data(std::uint64_t conn_id, T const &val) {
            std::shared_ptr<udp_connection> conn_ptr{connection_by_id(conn_id)};
            if(conn_ptr) {
                conn_ptr->set_user_data(val);
            } else {
                throw std::runtime_error{"connection not found"};
            }
        }

        template<typename T>
        T conn_user_data(std::uint64_t conn_id) const {
            std::shared_ptr<udp_connection> conn_ptr{connection_by_id(conn_id)};
            if(conn_ptr) {
                return conn_ptr->template user_data<T>();
            }
            throw std::runtime_error{"connection not found"};
        }

        bool stop() {
            bool res{false};
            try {
                terminate();
                wait();
                remove_all_connections();
                {
                    std::unique_lock l{sock_fd_mtp_};
                    if(sock_fd_ != -1) {
                        poller_.del_event(sock_fd_);
                        poller_.close();
                        ::close(sock_fd_);
                        sock_fd_ = -1;
                    }
                }
                started_ = false;
            } catch(...) {
            }
            return res;
        }

        bool ok() const {
            return sock_fd_ != -1;
        }

        void remove_all_connections() {
            std::unique_lock l{connections_mtp_};
            while(connections_.size()) {
                remove_conn_dont_send_message_unlocked(connections_.begin()->first);
            }
        }

        using in_struct = std::pair<std::array<std::uint8_t, NET_PACKET_PAYLOAD_SIZE_MAX + 256>, std::size_t>;
        void worker() {
            try {
                union {
                    struct sockaddr_in v4_;
                    struct sockaddr_in6 v6_;
                } cli_addr;
                std::memset(&cli_addr, 0, sizeof(cli_addr));

                ssize_t n{0};
                std::shared_ptr<in_struct> in_buff{std::make_shared<in_struct>()};
                std::vector<poll_event> events{poller_.wait(1, timespec_wrapper{0.1})};
                if(events.size() == 1) {
                    if((events[0].events & teal::net::POLL_EVENT_IN) == teal::net::POLL_EVENT_IN) {
                        std::shared_lock l{sock_fd_mtp_};
                        if(sock_fd_ >= 0) {
                            socklen_t socklen{(socklen_t)(sock_type_ == address_family::inet6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in))};
                            // std::cout << "srv: " << "calling recvfrom()" << std::endl;
                            n = ::recvfrom(sock_fd_, in_buff->first.data(), NET_PACKET_PAYLOAD_SIZE_MAX + 256, 0, (struct sockaddr *)&cli_addr, &socklen);
                            // std::cout << "srv: " << "recvfrom() returned " << n << std::endl;
                            if(n > 0) {
                                in_buff->second = n;
                                /*cq_->enqueue*/enqueue_job([this, cli_addr, in_buff]() {
                                    process_input(cli_addr.v4_, cli_addr.v6_, in_buff);
                                });
                            } else {
                                /*cq_->enqueue*/enqueue_job([this]() {
                                    stop();
                                });
                            }
                        }
                    } else {
                        /*cq_->enqueue*/enqueue_job([this]() {
                            stop();
                        });
                    }
                }
            } catch (...) {
                /*cq_->enqueue*/enqueue_job([this]() {
                    stop();
                });
            }
        }

        void wait() {
            std::unique_lock sl{threads_mtp_};
            for(auto &&t: threads_) {
                if(t.joinable()) {
                    t.join();
                }
            }
            threads_.clear();
        }

        bool started() const {
            return started_;
        }

        bool start(std::string const &addr, int port, int num_listen_threads) {
            bool res{false};
            std::unique_lock l{sock_fd_mtp_};
            if(started_) {
                res = true;
            } else {
                unterminate();
                if(sock_type_ == address_family::inet4) {
                    if((sock_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
                        if(!helpers::set_rcv_timeout(sock_fd_, recv_timeout_)) {
                            ::close(sock_fd_);
                            sock_fd_ = -1;
                        }
                        if(sock_fd_ != -1 && async_) {
                            if(!helpers::make_nonblocking(sock_fd_)) {
                                ::close(sock_fd_);
                                sock_fd_ = -1;
                            }
                        }
                        if(sock_fd_ != -1) {
                            try {
                                std::memset(&serv_addr_.v4_, 0, sizeof(serv_addr_.v4_));
                                serv_addr_.v4_.sin_family = AF_INET;
                                in_addr a{teal::net::resolve(addr)};
                                serv_addr_.v4_.sin_addr.s_addr = a.s_addr;
                                serv_addr_.v4_.sin_port = teal::bit_util::swap_on_le<decltype(serv_addr_.v4_.sin_port)>{static_cast<decltype(serv_addr_.v4_.sin_port)>(port)}.val;
                                if(::bind(sock_fd_, (const struct sockaddr *)&serv_addr_.v4_, sizeof(serv_addr_.v4_)) >= 0) {
                                    poller_.add_event(sock_fd_, teal::net::POLL_EVENT_IN);
                                    if(num_listen_threads <= 0) {
                                        num_listen_threads = 1;
                                    }
                                    std::unique_lock sl{threads_mtp_};
                                    for(int i = 0; i < num_listen_threads; ++i) {
                                        threads_.emplace_back([this]() {
                                            while(!termination()) {
                                                worker();
                                            }
                                        });
                                    }
                                    started_ = true;
                                    res = true;
                                }
                            } catch (...) {
                            }
                        }
                    }
                } else if(sock_type_ == address_family::inet6) {
                    if((sock_fd_ = ::socket(AF_INET6, SOCK_DGRAM, 0)) >= 0) {
                        if(!helpers::set_rcv_timeout(sock_fd_, recv_timeout_)) {
                            ::close(sock_fd_);
                            sock_fd_ = -1;
                        }
                        if(sock_fd_ != -1 && async_) {
                            if(!helpers::make_nonblocking(sock_fd_)) {
                                ::close(sock_fd_);
                                sock_fd_ = -1;
                            }
                        }
                        if(sock_fd_ != -1) {
                            try {
                                std::memset(&serv_addr_.v6_, 0, sizeof(serv_addr_.v6_));
                                serv_addr_.v6_.sin6_family = AF_INET6;
                                in6_addr a{teal::net::resolve6(addr)};
                                serv_addr_.v6_.sin6_addr = a;
                                serv_addr_.v6_.sin6_port = teal::bit_util::swap_on_le<decltype(serv_addr_.v6_.sin6_port)>{static_cast<decltype(serv_addr_.v6_.sin6_port)>(port)}.val;
                                if(::bind(sock_fd_, (const struct sockaddr *)&serv_addr_.v6_, sizeof(serv_addr_.v6_)) >= 0) {
                                    poller_.add_event(sock_fd_, teal::net::POLL_EVENT_IN);
                                    if(num_listen_threads <= 0) {
                                        num_listen_threads = 1;
                                    }
                                    std::unique_lock sl{threads_mtp_};
                                    for(int i = 0; i < num_listen_threads; ++i) {
                                        threads_.emplace_back([this]() {
                                            while(!termination()) {
                                                worker();
                                            }
                                        });
                                    }
                                    started_ = true;
                                    res = true;
                                }
                            } catch (...) {
                            }
                        }
                    }
                } else {
                }
            }
            return res;
        }

        bool send(std::uint64_t conn_id, std::vector<std::uint8_t> const &data) {
            return send(conn_id, data.data(), data.size());
        }

        bool send(std::uint64_t conn_id, std::string const &data) {
            return send(conn_id, data.data(), data.size());
        }

        bool send(std::uint64_t conn_id, void const *data, std::size_t data_size) {
            bool res{false};
            if(data != nullptr && data_size && sock_fd_ >= 0) {
                try {
                    std::shared_ptr<udp_connection> cnn{connection_by_id(conn_id)};
                    if(cnn) {
                        teal::serializer ser{};
                        if(cnn->encryption_enabled()) {
                            std::uint64_t ctr_start{cnn->crypto_ctr_for_data_size(data_size)};
                            ser << (std::uint8_t)1 << ctr_start << cnn->encrypt_data(data, data_size, ctr_start);
                        } else {
                            ser << (std::uint8_t)0;
                            ser.push_back(data, data_size);
                        }
                        cnn->set_output_message(ser.data_vec());
                        while(auto och{cnn->fetch_out_chunk()}) {
                            std::shared_lock l{sock_fd_mtp_};
                            if(sock_fd_ >= 0) {
                                socklen_t socklen{(socklen_t)(sock_type_ == address_family::inet6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in))};
                                res = ::sendto(sock_fd_, och->data(), och->size(), 0, (const struct sockaddr *)cnn->addr_ptr(), socklen) != -1;
                            }
                        }
                    }
                } catch (...) {
                    res = false;
                }
            }
            return res;
        }

        bool send_plain(std::uint64_t conn_id, std::vector<std::uint8_t> const &data) {
            return send_plain(conn_id, data.data(), data.size());
        }

        bool send_plain(std::uint64_t conn_id, std::string const &data) {
            return send_plain(conn_id, data.data(), data.size());
        }

        bool send_plain(std::uint64_t conn_id, void const *data, std::size_t data_size) {
            bool res{false};
            if(data != nullptr && data_size && sock_fd_ >= 0) {
                try {
                    std::shared_ptr<udp_connection> cnn{connection_by_id(conn_id)};
                    if(cnn) {
                        teal::serializer ser{};
                        ser << (std::uint8_t)0;
                        ser.push_back(data, data_size);
                        cnn->set_output_message(ser.data_vec());
                        teal::bytevec ch;
                        while(cnn->fetch_out_chunk(ch)) {
                            std::shared_lock l{sock_fd_mtp_};
                            if(sock_fd_ >= 0) {
                                socklen_t socklen{(socklen_t)(sock_type_ == address_family::inet6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in))};
                                res = ::sendto(sock_fd_, ch.data(), ch.size(), 0, (const struct sockaddr *)cnn->addr_ptr(), socklen) != -1;
                            }
                        }
                    }
                } catch (...) {
                    res = false;
                }
            }
            return res;
        }

        bool remove_conn(std::uint64_t conn_id) {
            bool res{false};
            std::unique_lock csl{connections_mtp_};
            auto it{connections_.find(conn_id)};
            if(it != connections_.end()) {
                std::shared_ptr<udp_connection> cnn{it->second};
                connections_.erase(it);
                csl.unlock();
                {
                    std::shared_lock l{sock_fd_mtp_};
                    if(sock_fd_ >= 0) {
                        socklen_t socklen{(socklen_t)(sock_type_ == address_family::inet6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in))};
                        ::sendto(sock_fd_, "close", 5, MSG_DONTWAIT, cnn->addr_ptr(), socklen);
                    }
                }
                if(on_connection_closed_) {
                    on_connection_closed_(conn_id);
                }
                res = true;
            }
            return res;
        }

        std::shared_ptr<udp_connection> connection_by_id(std::uint64_t conn_id) const {
            std::shared_lock l{connections_mtp_};
            auto it{connections_.find(conn_id)};
            return it != connections_.end() ? it->second : std::shared_ptr<udp_connection>{};
        }

        bool connection_exists(std::uint64_t conn_id) const {
            std::shared_lock l{connections_mtp_};
            return connections_.find(conn_id) != connections_.end();
        }

        // void recheck_conn_timeouts_and_clean(long double seconds) {
        //     std::map<std::uint64_t, std::shared_ptr<udp_connection>> connections_copy{**connections_};
        //     for(auto &&p: connections_copy) {
        //         if(p.second->seconds_since_last_activity() > seconds) {
        //             remove_conn(p.first);
        //         }
        //     }
        // }

    private:
        // struct sockaddr_in conn_addr(std::uint64_t conn_id) const {
        //     std::shared_lock l{connections_mtp_};
        //     return connections_--->at(conn_id)->addr();
        // }

        bool remove_conn_dont_send_message_unlocked(std::uint64_t conn_id) {
            bool res{false};
            auto it{connections_.find(conn_id)};
            if(it != connections_.end()) {
                connections_.erase(conn_id);
                if(on_connection_closed_) {
                    on_connection_closed_(conn_id);
                }
                res = true;
            }
            return res;
        }

        bool remove_conn_dont_send_message(std::uint64_t conn_id) {
            bool res{false};
            std::unique_lock csl{connections_mtp_};
            auto it{connections_.find(conn_id)};
            if(it != connections_.end()) {
                connections_.erase(conn_id);
                csl.unlock();
                if(on_connection_closed_) {
                    on_connection_closed_(conn_id);
                }
                res = true;
            }
            return res;
        }

        void process_new_connection(
            sockaddr_in cliaddr4,
            sockaddr_in6 cliaddr6
        ) {
            // std::cout << "srv: " << "new conn" << std::endl;
            teal::serializer ser{};
            std::uint64_t conn_id{};
            conn_id = conn_id_gen_();

            std::shared_ptr<udp_connection> conn{std::make_shared<udp_connection>(cliaddr4, cliaddr6, sock_type_, conn_id)};
            {
                std::unique_lock l{connections_mtp_};
                conn->update_last_activity_time();
                connections_[conn_id] = conn;
            }
            if(on_new_connection_) {
                // std::cout << "srv: " << "reporting new conn" << std::endl;
                on_new_connection_(conn_id);
            }
            ser << "ok" << conn_id;
            socklen_t socklen{(socklen_t)(sock_type_ == address_family::inet6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in))};
            std::shared_lock sl{sock_fd_mtp_};
            if(sock_fd_ >= 0) {
                // std::cout << "srv: " << "ok to client: " << ser.data_vec() << std::endl;
                if(::sendto(sock_fd_, ser.data_vec().data(), ser.data_vec().size(), 0, conn->addr_ptr(), socklen) == -1) {
                    // std::cout << "srv: " << "send failed" << std::endl;
                    sl.unlock();
                    std::unique_lock l{connections_mtp_};
                    connections_.erase(conn_id);
                }
            } else {
                // std::cout << "srv: " << "invalid socket" << std::endl;
            }
        }

        void process_input(struct sockaddr_in cliaddr4, struct sockaddr_in6 cliaddr6, std::shared_ptr<in_struct> in_buff) {
            std::size_t insize{in_buff->second};
            if(insize == 3) {
                if(std::string{in_buff->first.begin(), in_buff->first.begin() + 3} == "con") {
                    process_new_connection(cliaddr4, cliaddr6);
                }
            } else if(insize == 13 && std::string{in_buff->first.begin(), in_buff->first.begin() + 5} == "close") {
                std::uint64_t conn_id{};
                std::memcpy(&conn_id, in_buff->first.data() + 5, 8);
                remove_conn_dont_send_message(conn_id);
            } else if(insize > 0 && on_data_from_client_ != nullptr) {
                std::uint64_t conn_id{};
                std::memcpy(&conn_id, in_buff->first.data(), 8);
                std::shared_lock conn_lck{connections_mtp_};
                auto conn_desc_it{connections_.find(conn_id)};
                if(conn_desc_it != connections_.end()) {
                    std::shared_ptr<udp_connection> conn_ptr{conn_desc_it->second};
                    conn_lck.unlock();
                    std::optional<teal::bytevec> msg{conn_ptr->set_incoming_data(in_buff->first.data() + 8, in_buff->first.size() - 8)};
                    if(msg) {
                        teal::serial_reader const ser{msg->data(), msg->size()};
                        teal::serial_reader::const_iterator iter{ser.cbegin()};
                        if(iter->as_unumber() == 0) {
                            ++iter;
                            on_data_from_client_(conn_id, iter->data(), iter->size());
                        } else {
                            ++iter;
                            std::uint64_t ctr_start{iter->as_unumber()};
                            ++iter;
                            std::vector<std::uint8_t> d{conn_ptr->decrypt_data(iter->data(), iter->size(), ctr_start)};
                            on_data_from_client_(conn_id, d.data(), d.size());
                        }
                    }
                }
            }
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

        bool enqueue_job(std::function<void()> &&fn) {
            if(stop_jobs_) { return false; }
            std::unique_lock l{jobs_buffer_mtp_};
            jobs_buffer_.push_back(std::move(fn));
            jobs_buffer_cvar_.notify_all();
            return true;
        }

        bool enqueue_job(std::function<void()> const &fn) {
            if(stop_jobs_) { return false; }
            std::unique_lock l{jobs_buffer_mtp_};
            jobs_buffer_.push_back(fn);
            jobs_buffer_cvar_.notify_all();
            return true;
        }

    private:
        mutable std::shared_mutex threads_mtp_{};
        std::list<std::jthread> threads_{};

        std::atomic_bool stop_jobs_{false};
        mutable std::mutex jobs_buffer_mtp_{};
        std::condition_variable jobs_buffer_cvar_{};
        mutable std::mutex jobs_buffer_workers_mtp_{};
        std::list<std::thread> jobs_buffer_workers_{};
        std::list<std::function<void()>> jobs_buffer_{};

        union {
            struct sockaddr_in v4_;
            struct sockaddr_in6 v6_;
        } serv_addr_;
        teal::atomic_sequence_generator<std::uint64_t> conn_id_gen_{1};
        mutable std::shared_mutex connections_mtp_{};
        mutable std::map<std::uint64_t, std::shared_ptr<udp_connection>> connections_{};
        std::function<void(std::uint64_t)> on_new_connection_{nullptr};
        std::function<void(std::uint64_t, void const *, std::size_t)> on_data_from_client_{nullptr};
        std::function<void(std::uint64_t)> on_connection_closed_{nullptr};
        long double recv_timeout_{0.1L};
        mutable std::shared_mutex sock_fd_mtp_{};
        int sock_fd_{-1};
        socket_poller poller_{};
        address_family sock_type_{address_family::unspecified};
        bool async_{false};
        bool started_{false};
    };

}
