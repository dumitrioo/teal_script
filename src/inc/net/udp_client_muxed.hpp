#pragma once

#include "../commondefs.hpp"
#include "../timespec_wrapper.hpp"
#include "../serialization.hpp"
#include "../terminable.hpp"
#include "../crypto/gamma.hpp"
#include "../sequence_generator.hpp"
#include "net_utils.hpp"
#include "socket_wrapper.hpp"
#include "net_data_transfer.hpp"
#include "udp_hlp.hpp"

namespace teal::net {

    template<std::size_t NET_PACKET_PAYLOAD_SIZE_MAX = 1450>
    class udp_client_muxed {
    public:
        udp_client_muxed(
            long double recv_timeout = 0.1L,
            bool async = false,
            address_family af = address_family::inet4
        ):
            recv_timeout_{recv_timeout},
            sock_type_{af},
            async_{async}
        {
        }

        udp_client_muxed(udp_client_muxed const &) = delete;
        udp_client_muxed(udp_client_muxed &&) = delete;
        udp_client_muxed &operator=(udp_client_muxed const &) = delete;
        udp_client_muxed &operator=(udp_client_muxed &&) = delete;
        ~udp_client_muxed() { close(); }

        bool connected() const {
            return sock_fd_ != -1;
        }

        bool connect(std::string const &addr, std::uint16_t port) {
            bool result{false};
            std::unique_lock l{sock_fd_mtp_};
            if(sock_fd_ != -1) {
                result = true;
            } else {
                std::string const conn_data{"con"};
                if(sock_type_ == address_family::inet4) {
                    if((sock_fd_ = ::socket(AF_INET, SOCK_DGRAM, 0)) != -1) {
                        if(async_) {
                            if(!helpers::make_nonblocking(sock_fd_)) {
                                ::close(sock_fd_);
                                sock_fd_ = -1;
                            }
                        }
                        if(sock_fd_ != -1) {
                            try {
                                if(helpers::set_rcv_timeout(sock_fd_, 15)) {
                                    in_addr dst_ip{teal::net::resolve(addr)};
                                    std::memset(&serv_addr_.v4_, 0, sizeof(serv_addr_.v4_));
                                    serv_addr_.v4_.sin_family = AF_INET;
                                    serv_addr_.v4_.sin_port = ::htons(port);
                                    serv_addr_.v4_.sin_addr.s_addr = dst_ip.s_addr;
                                    // std::cout << "cli: " << "sending " << conn_data << std::endl;
                                    socklen_t socklen{(socklen_t)(sock_type_ == address_family::inet6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in))};
                                    if(::sendto(sock_fd_, conn_data.data(), conn_data.size(), 0, serv_addr(), socklen) != -1) {
                                        std::array<std::uint8_t, NET_PACKET_PAYLOAD_SIZE_MAX + 256> buffer{};
                                        ssize_t n = ::recvfrom(sock_fd_, buffer.data(), buffer.size(), 0, serv_addr(), &socklen);
                                        helpers::set_rcv_timeout(sock_fd_, recv_timeout_);
                                        if(n > 0) {
                                            teal::serial_reader sr{buffer.data(), (std::size_t)n};
                                            // std::cout << "cli: " << "received " << sr.data_vec() << std::endl;
                                            teal::serial_reader::const_iterator it{sr.cbegin()};
                                            if(it->as_string() == "ok") {
                                                ++it;
                                                conn_id_ = it->as_unumber();
                                                result = true;
                                            }
                                        }
                                    }
                                }
                            } catch (...) {
                            }
                        }
                    }
                } else if(sock_type_ == address_family::inet6) {
                    if((sock_fd_ = ::socket(AF_INET6, SOCK_DGRAM, 0)) != -1) {
                        if(async_) {
                            if(!helpers::make_nonblocking(sock_fd_)) {
                                ::close(sock_fd_);
                                sock_fd_ = -1;
                            }
                        }
                        if(sock_fd_ != -1) {
                            try {
                                if(helpers::set_rcv_timeout(sock_fd_, 15)) {
                                    in6_addr dst_ip{teal::net::resolve6(addr)};
                                    std::memset(&serv_addr_.v6_, 0, sizeof(serv_addr_.v6_));
                                    serv_addr_.v6_.sin6_family = AF_INET6;
                                    serv_addr_.v6_.sin6_port = ::htons(port);
                                    serv_addr_.v6_.sin6_addr = dst_ip;
                                    socklen_t socklen{(socklen_t)(sock_type_ == address_family::inet6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in))};
                                    if(::sendto(sock_fd_, conn_data.data(), conn_data.size(), 0, serv_addr(), socklen) != -1) {
                                        std::array<std::uint8_t, NET_PACKET_PAYLOAD_SIZE_MAX + 256> buffer{};
                                        ssize_t n = ::recvfrom(sock_fd_, buffer.data(), buffer.size(), 0, serv_addr(), &socklen);
                                        helpers::set_rcv_timeout(sock_fd_, recv_timeout_);
                                        if(n > 0) {
                                            teal::serial_reader sr{buffer.data(), (std::size_t)n};
                                            teal::serial_reader::const_iterator it{sr.cbegin()};
                                            if(it->as_string() == "ok") {
                                                ++it;
                                                conn_id_ = it->as_unumber();
                                                result = true;
                                            }
                                        }
                                    }
                                }
                            } catch (...) {
                            }
                        }
                    }
                } else {
                }
            }
            if(!result) {
                sock_fd_ = -1;
                sock_type_ = address_family::unspecified;
                conn_id_ = 0;
            }
            return result;
        }

        bool ok() const {
            std::shared_lock l{sock_fd_mtp_};
            return sock_fd_ >= 0 && (sock_type_ == address_family::inet4 || sock_type_ == address_family::inet6);
        }

        void close() {
            std::vector<std::uint8_t> close_data{'c', 'l', 'o', 's', 'e', 0, 0, 0, 0, 0, 0, 0, 0};
            std::unique_lock l{sock_fd_mtp_};
            if(sock_fd_ >= 0) {
                std::memcpy(&close_data[5], &conn_id_, 8);
                ::sendto(sock_fd_, close_data.data(), close_data.size(), MSG_DONTWAIT, serv_addr(), sizeof(struct sockaddr_in));
                ::close(sock_fd_);
                sock_fd_ = -1;
                sock_type_ = address_family::unspecified;
            }
        }

        bool send_plain(std::vector<std::uint8_t> const &d) {
            teal::serializer ser{};
            ser << (std::uint8_t)0 << d;
            std::unique_lock l{muxer_mtp_};
            muxer_.add_message(ser.data_vec());
            std::array<std::uint8_t, NET_PACKET_PAYLOAD_SIZE_MAX + 256> buff{};
            while(std::optional<bytevec> och{muxer_.fetch_out_chunk()}) {
                teal::bytevec &ch{*och};
                std::memcpy(&buff[0], &conn_id_, 8);
                std::memcpy(&buff[8], ch.data(), ch.size());
                if(::sendto(sock_fd_, buff.data(), ch.size() + 8, MSG_DONTWAIT, serv_addr(), sizeof(struct sockaddr_in)) == -1) {
                    ::close(sock_fd_);
                    sock_fd_ = -1;
                    sock_type_ = address_family::unspecified;
                    return false;
                }
            }
            return true;
        }

        bool send_plain(std::string const &data) {
            return send_plain(std::vector<std::uint8_t>{data.begin(), data.end()});
        }

        bool send(std::vector<std::uint8_t> const &d) {
            teal::serializer ser{};
            if(encryption_on_) {
                std::uint64_t ctr_start{crypto_ctr_.fetch_add(d.size())};
                ser << (std::uint8_t)1 << ctr_start << encrypt_data(d, ctr_start);
            } else {
                ser << (std::uint8_t)0 << d;
            }
            {
                std::unique_lock l{muxer_mtp_};
                muxer_.add_message(ser.data_vec());
            }
            teal::bytevec ch{};
            std::array<std::uint8_t, NET_PACKET_PAYLOAD_SIZE_MAX + 256> buff{};
            while(true) {
                {
                    std::unique_lock l{muxer_mtp_};
                    auto och{muxer_.fetch_out_chunk()};
                    if(!och) {
                        break;
                    }
                    ch = std::move(*och);
                }
                std::memcpy(&buff[0], &conn_id_, 8);
                std::memcpy(&buff[8], ch.data(), ch.size());
                if(::sendto(sock_fd_, buff.data(), ch.size() + 8, MSG_DONTWAIT, serv_addr(), sizeof(struct sockaddr_in)) == -1) {
                    ::close(sock_fd_);
                    sock_fd_ = -1;
                    sock_type_ = address_family::unspecified;
                    return false;
                }
            }
            return true;
        }

        bool send(std::string const &data) {
            return send(std::vector<std::uint8_t>{data.begin(), data.end()});
        }

        std::optional<std::vector<std::uint8_t>> receive() {
            std::optional<std::vector<std::uint8_t>> result{};
            std::shared_lock l{sock_fd_mtp_};
            if(sock_fd_ >= 0) {
                std::array<std::uint8_t, NET_PACKET_PAYLOAD_SIZE_MAX + 256> buffer{};
                socklen_t socklen{(socklen_t)(sock_type_ == address_family::inet6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in))};
                ssize_t n = ::recvfrom(sock_fd_, buffer.data(), buffer.size(), 0, serv_addr(), &socklen);
                l.unlock();
                if(n == 5 && std::memcmp("close", buffer.data(), 5) == 0) {
                    ::close(sock_fd_);
                    sock_fd_ = -1;
                    sock_type_ = address_family::unspecified;
                } else if(n > 0) {
                    teal::serial_reader const ser{buffer.data(), (teal::serial_reader::size_type)n};
                    teal::serial_reader::const_iterator iter{ser.cbegin()};
                    if(iter->as_unumber() == 0) {
                        ++iter;
                        std::unique_lock l{demuxer_mtp_};
                        result = demuxer_.add_data(iter->data(), iter->size());
                        remove_stale_inputs_unlocked(120);
                    } else {
                        ++iter;
                        std::uint64_t ctr_start{iter->as_unumber()};
                        ++iter;
                        std::vector<std::uint8_t> d{decrypt_data(iter->data(), iter->size(), ctr_start)};
                        std::unique_lock l{demuxer_mtp_};
                        result = demuxer_.add_data(d);
                        remove_stale_inputs_unlocked(120);
                    }
                }
            }
            return result;
        }

    private:
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
        std::uint64_t conn_id_{0};
        long double recv_timeout_{15.0L};
        union {
            struct sockaddr_in v4_;
            struct sockaddr_in6 v6_;
        } serv_addr_;
        mutable std::shared_mutex sock_fd_mtp_{};
        int sock_fd_{-1};
        address_family sock_type_{address_family::unspecified};
        bool async_{false};

        struct sockaddr const *serv_addr() const {
            return (struct sockaddr const *)&serv_addr_;
        }

        struct sockaddr *serv_addr() {
            return (struct sockaddr *)&serv_addr_;
        }

    private:
        std::mutex encrypt_mtp_{};
        std::mutex decrypt_mtp_{};
        std::atomic<std::uint64_t> crypto_ctr_{0};
        teal::crypt::gamma out_gamma_{};
        teal::crypt::gamma in_gamma_{};
        std::atomic<bool> encryption_on_{false};

        std::vector<std::uint8_t> encrypt_data(std::vector<std::uint8_t> const &p, std::uint64_t ctr_start) {
            std::unique_lock lck{encrypt_mtp_};
            teal::crypt::gamma out_gamma{out_gamma_};
            lck.unlock();
            std::vector<std::uint8_t> e{p};
            std::uint64_t crypto_ctr{ctr_start};
            for(std::size_t i{0}; i < e.size(); ++i) {
                e[i] = e[i] ^ out_gamma[crypto_ctr++];
            }
            return e;
        }

        std::vector<std::uint8_t> decrypt_data(void const *e, std::size_t es, std::uint64_t ctr_start) {
            if(!e || !es) {
                return {};
            }
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

    public:
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
    };

}
