#pragma once

#include "../commondefs.hpp"
#include "../timespec_wrapper.hpp"
#include "../bit_util.hpp"
#include "../sys_util.hpp"
#include "net_utils.hpp"
#ifndef PLATFORM_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <sys/un.h>
#elif defined(PLATFORM_POSIX) || defined(PLATFORM_UNIXISH) || defined(PLATFORM_APPLE)
#include <netinet/tcp.h>
#endif

namespace teal::net {

    DEFINE_RUNTIME_ERROR_CLASS(socket_error)

    class socket final {
    public:
        socket() = default;
        socket(socket const &) = delete;
        socket &operator=(socket const &) = delete;
        socket(socket &&other) noexcept:
            sock_fd_{other.sock_fd_},
            sock_type_{other.sock_type_},
            common_addr_{other.common_addr_}
        {
            other.sock_fd_ = -1;
            sock_type_ = address_family::unspecified;
        }
        socket &operator=(socket &&other) noexcept {
            if(this != &other) {
                sock_fd_ = other.sock_fd_;
                sock_type_ = other.sock_type_;
                common_addr_ = other.common_addr_;

                other.sock_fd_ = -1;
                sock_type_ = address_family::unspecified;
            }
            return *this;
        }
        ~socket() {
            try {
                shutdown();
            } catch(...) {
            }
            close();
        }
        bool create(address_family domain = address_family::inet4, sock_type type = sock_type::stream, int proto = 0) {
            if(ok()) {
                throw socket_error{"socket::create() call on already initialized socket"};
            }
            if(domain == address_family::inet4) {
                if((sock_fd_ = ::socket(AF_INET, (int)type, proto)) > 0) {
                    sock_type_ = domain;
                    return true;
                }
            } else if(domain == address_family::inet6) {
                if((sock_fd_ = ::socket(AF_INET6, (int)type, proto)) > 0) {
                    sock_type_ = domain;
                    return true;
                }
            }
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            else if(domain == address_family::unix_socket) {
                if((sock_fd_ = ::socket(AF_UNIX, (int)type, proto)) > 0) {
                    sock_type_ = domain;
                    return true;
                }
            }
#endif
            return false;
        }
        bool bind(const std::string &address, std::uint16_t port) {
            if(ok()) {
                if(sock_type_ == address_family::inet4) {
                    common_addr_.sock_addr_.sin_family = AF_INET;
                    common_addr_.sock_addr_.sin_port = teal::bit_util::hnswap<std::uint16_t>{static_cast<std::uint16_t>(port)}.val;
                    common_addr_.sock_addr_.sin_addr = teal::net::pton(address);
                    if(::bind(sock_fd_, (struct sockaddr *)&common_addr_.sock_addr_, sizeof(struct sockaddr)) == 0) {
                        return true;
                    }
                } else if(sock_type_ == address_family::inet6) {
                    common_addr_.sock_addr6_.sin6_family = AF_INET6;
                    common_addr_.sock_addr6_.sin6_port = teal::bit_util::hnswap<std::uint16_t>{static_cast<std::uint16_t>(port)}.val;
                    common_addr_.sock_addr6_.sin6_addr = teal::net::pton6(address);
                    if(::bind(sock_fd_, (struct sockaddr *)&common_addr_.sock_addr6_, sizeof(struct sockaddr)) == 0) {
                        return true;
                    }
                }
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
                else if(sock_type_ == address_family::unix_socket) {
                    common_addr_.sock_addr_unix_.sun_family = AF_UNIX;
                    if(file_util::file_exists(address)) {
                        ::unlink(address.c_str());
                    }
                    strcpy(common_addr_.sock_addr_unix_.sun_path, address.c_str());
                    if(::bind(sock_fd_, (struct sockaddr *)&common_addr_.sock_addr6_, sizeof(struct sockaddr)) == 0) {
                        unix_server_socket_path_ = address;
                        return true;
                    }
                }
#endif
            }
            return false;
        }
        bool listen(int que = SOMAXCONN) {
            if(ok()) {
                if(::listen(sock_fd_, que) == 0) {
                    return true;
                }
            } else {
                throw socket_error{"socket::listen() call on uninitialized socket"};
            }
            return false;
        }
        void accept(socket &client_sock) {
            if(ok()) {
                socklen_t size{sizeof(struct sockaddr)};
                if(sock_type_ == address_family::inet4) {
                    size = sizeof(client_sock.common_addr_.sock_addr_);
                    client_sock.sock_fd_ = ::accept(sock_fd_, (struct sockaddr *)&client_sock.common_addr_.sock_addr_, &size);
                    client_sock.sock_type_ = address_family::inet4;
                } else if(sock_type_ == address_family::inet6) {
                    size = sizeof(client_sock.common_addr_.sock_addr6_);
                    client_sock.sock_fd_ = ::accept(sock_fd_, (struct sockaddr *)&client_sock.common_addr_.sock_addr6_, &size);
                    client_sock.sock_type_ = address_family::inet6;
                }
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
                else if(sock_type_ == address_family::unix_socket) {
                    size = sizeof(client_sock.common_addr_.sock_addr_unix_);
                    client_sock.sock_fd_ = ::accept(sock_fd_, (struct sockaddr *)&client_sock.common_addr_.sock_addr_unix_, &size);
                    client_sock.sock_type_ = address_family::unix_socket;
                }
#endif
                else {
                    throw socket_error{"socket::accept() unspecified socket type"};
                }
                if(client_sock.sock_fd_ == -1) {
                    client_sock.sock_type_ = address_family::unspecified;
                    throw socket_error{teal::sys_util::error_str(teal::sys_util::last_error())};
                }
            } else {
                throw socket_error{"socket::accept() call on uninitialized socket"};
            }
        }
        bool connect(const std::string &address, std::uint16_t port) {
            if(ok()) {
                if(sock_type_ == address_family::inet4) {
                    struct hostent *hostPtr{::gethostbyname2(address.c_str(), AF_INET)};
                    if(hostPtr == NULL) {
                        return false;
                    }
                    struct in_addr *addr_ptr{(struct in_addr *)*hostPtr->h_addr_list};
                    if(addr_ptr) {
                        std::string add{ntop(*addr_ptr)};
                        if(add.empty()) {
                            return false;
                        }
                        struct sockaddr_in sock_addr;
                        sock_addr.sin_family = AF_INET;
                        sock_addr.sin_port = teal::bit_util::hnswap<std::uint16_t>{static_cast<std::uint16_t>(port)}.val;
                        sock_addr.sin_addr.s_addr = inet_addr(add.c_str());
                        int conn_res{::connect(sock_fd_, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr))};
                        if(conn_res == 0) {
                            return true;
                        }
                    }
                } else if(sock_type_ == address_family::inet6) {
                    struct hostent *hostPtr{::gethostbyname2(address.c_str(), AF_INET6)};
                    if(hostPtr == NULL) {
                        return false;
                    }
                    struct in6_addr *addr_ptr{(struct in6_addr *)*hostPtr->h_addr_list};
                    if(addr_ptr) {
                        std::string add{ntop(*addr_ptr)};
                        if(add.empty()) {
                            return false;
                        }
                        struct sockaddr_in6 sock_addr;
                        sock_addr.sin6_family = AF_INET6;
                        sock_addr.sin6_port = teal::bit_util::hnswap<std::uint16_t>{static_cast<std::uint16_t>(port)}.val;
                        sock_addr.sin6_addr = teal::net::pton6(add);
                        int conn_res{::connect(sock_fd_, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr))};
                        if(conn_res == 0) {
                            return true;
                        }
                    }
                }
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
                else if(sock_type_ == address_family::unix_socket) {
                    struct sockaddr_un sock_addr;
                    sock_addr.sun_family = AF_UNIX;
                    strcpy(sock_addr.sun_path, address.c_str());
                    int conn_res{::connect(sock_fd_, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr))};
                    if(conn_res == 0) {
                        return true;
                    }
                }
#endif
                else {
                    throw socket_error{"unspecified socket type"};
                }
            } else {
                throw socket_error{"socket::connect() call on uninitialized socket"};
            }
            return false;
        }
        int bytes_available() const {
            int res{-1};
            if(ok()) {
#if defined(PLATFORM_LINUX)
                if(ioctl(sock_fd_, FIONREAD, &res) == -1) {
                    res = -1;
                }
#elif defined(PLATFORM_WINDOWS)
                u_long rdavl = 0;
                if(ioctlsocket(sock_fd_, FIONREAD, &rdavl) == NO_ERROR) {
                    res = rdavl;
                }
#endif
            }
            return res;
        }
        std::vector<std::uint8_t> receive(int len) {
            std::vector<std::uint8_t> result{};
            if(ok()) {
                if(len <= 0) {
                    throw socket_error("wrong arguments in call to socket::receive()");
                }
                result.resize(len);
                int rd = recv(sock_fd_, reinterpret_cast<char *>(&result[0]), len, 0);
                if(rd >= 0) {
                    if(rd != len) { result.resize(rd); }
                } else {
                    throw socket_error{teal::sys_util::error_str(teal::sys_util::last_error())};
                }
            } else {
                throw socket_error("socket::receive(): socket not ready");
            }
            return result;
        }
        int send(const void *buff, int len, int flags = MSG_NOSIGNAL) {
            if(ok()) {
                if(len <= 0 || !buff) {
                    throw socket_error("wrong arguments in call to socket::send()");
                }
                return ::send(sock_fd_, reinterpret_cast<const char *>(buff), len, flags);
            } else {
                throw socket_error("socket::send(): socket not ready");
            }
        }
        int write(const void *buff, int len) {
            if(len <= 0 || !buff) {
                throw socket_error("wrong arguments in call to socket::send()");
            }
            if(ok()) {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
                struct iovec iov;
                iov.iov_base = (void *)buff;
                iov.iov_len = len;
                return ::writev(sock_fd_, &iov, 1);
#else
                return this->send(buff, len);
#endif
            } else {
                throw socket_error("socket::send(): socket not ready");
            }
        }
        bool close() noexcept {
            int close_res{-1};
            if(ok()) {
#ifdef PLATFORM_WINDOWS
                close_res = ::closesocket(sock_fd_);
#else
                close_res = ::close(sock_fd_);
#endif
                if(close_res == 0) {
                    sock_fd_ = -1;
                    sock_type_ = address_family::unspecified;
                }
            }
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            if(sock_type_ == address_family::unix_socket && !unix_server_socket_path_.empty()) {
                ::unlink(unix_server_socket_path_.c_str());
                unix_server_socket_path_.clear();
            }
#endif
            return close_res == 0;
        }
        bool shutdown() {
            if(ok()) {
                return ::shutdown(sock_fd_,
#ifdef PLATFORM_WINDOWS
                    0x02 /*SD_BOTH*/
#else
                    SHUT_RDWR
#endif
                ) == 0;
            }
            return false;
        }
        int handle() const noexcept {
            return sock_fd_;
        }
        operator int() const noexcept {
            return handle();
        }
        bool make_nonblocking() {
            if(!ok()) {
                throw socket_error("socket::make_nonblocking(): socket not ready");
            }
#ifdef PLATFORM_WINDOWS
            // Set the socket I/O mode: In this case FIONBIO
            // enables or disables the blocking mode for the
            // socket based on the numerical value of iMode.
            // If iMode = 0, blocking is enabled;
            // If iMode != 0, non-blocking mode is enabled.
            u_long iMode = 1;
            return ioctlsocket(sock_fd_, FIONBIO, &iMode) == NO_ERROR;
#else
            return set_fd_flag(O_NONBLOCK);
#endif
        }
        bool set_reuse_addr() {
            if(!ok()) {
                throw socket_error("socket::set_reuse_addr(): socket not ready");
            }
#ifdef PLATFORM_WINDOWS
            int val{ 1 };
            return ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, (char const *)&val, sizeof(int)) >= 0;
#else
            int val{1};
            return ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) >= 0;
#endif
        }
        bool make_nodelay() {
            if(!ok()) {
                throw socket_error("socket::make_nodelay(): socket not ready");
            }
#ifdef PLATFORM_WINDOWS
            BOOL b = TRUE;
            return ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_NODELAY, (char const *)&b, sizeof(BOOL)) == 0;
#else
            int one = 1;
            return ::setsockopt(sock_fd_, SOL_TCP, TCP_NODELAY, &one, sizeof(one)) == 0;
#endif
        }
        bool cork(bool val) {
            if(!ok()) {
                throw socket_error("socket::cork(): socket not ready");
            }
#ifdef PLATFORM_WINDOWS
            return false;
#else
            int one_or_zero{val ? 1 : 0};
            return ::setsockopt(sock_fd_, SOL_TCP, TCP_CORK, &one_or_zero, sizeof(one_or_zero)) == 0;
#endif
        }
        bool make_nosigpipe() {
            if(!ok()) {
                throw socket_error("socket::make_nosigpipe(): socket not ready");
            }
#ifdef PLATFORM_APPLE
            int opt = 1;
            return ::setsockopt(sock_fd_, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt)) != -1;
#else
            return true;
#endif
        }
        bool set_linger(bool on, int linger_time) {
            if(!ok()) {
                throw socket_error("socket::set_linger(): socket not ready");
            }
            struct linger sl{};
            sl.l_onoff = on ? 1 : 0;
            sl.l_linger = linger_time;
            return ::setsockopt(sock_fd_, SOL_SOCKET, SO_LINGER, (char const *)&sl, sizeof(sl)) == 0;
        }
        bool set_keepalive(bool keep) {
            if(!ok()) {
                throw socket_error("socket::set_keepalive(): socket not ready");
            }
            int flags{keep ? 1 : 0};
            return ::setsockopt(sock_fd_, SOL_SOCKET, SO_KEEPALIVE, (char const *)&flags, sizeof(flags)) == 0;
        }
        bool get_keepalive() const {
            if(!ok()) {
                throw socket_error("socket::get_keepalive(): socket not ready");
            }
            socklen_t len = sizeof(int);
            int flags{};
            if(::getsockopt(sock_fd_, SOL_SOCKET, SO_KEEPALIVE, (char *)&flags, &len) != 0) {
                throw socket_error("socket::get_keepalive(): failed to query option");
            }
            return flags;
        }
        bool set_tcp_keepidle(int val) {
            if(!ok()) {
                throw socket_error("socket::set_tcp_keepidle(int): socket not ready");
            }
#ifndef PLATFORM_APPLE
            int idle = val;
            return ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPIDLE, (char const *)&idle, sizeof(int)) == 0;
#else
            return true;
#endif
        }
        int get_tcp_keepidle() const {
            if(!ok()) {
                throw socket_error("socket::get_tcp_keepidle(): socket not ready");
            }
#ifndef PLATFORM_APPLE
            socklen_t len = sizeof(int);
            int val{};
            if(::getsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPIDLE, (char *)&val, &len) != 0) {
                throw socket_error("socket::get_tcp_keepidle(): failed to query option");
            }
            return val;
#else
            return -1;
#endif
        }
        bool set_tcp_keepitvl(int val) {
            if(!ok()) {
                throw socket_error("socket::set_tcp_keepitvl(): socket not ready");
            }
            int interval = val;
            return ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPINTVL, (char const *)&interval, sizeof(int)) == 0;
        }
        int get_tcp_keepitvl() const {
            if(!ok()) {
                throw socket_error("socket::get_tcp_keepitvl(): socket not ready");
            }
            socklen_t len = sizeof(int);
            int val{};
            if(::getsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPINTVL, (char *)&val, &len) != 0) {
                throw socket_error("socket::get_tcp_keepitvl(): failed to query option");
            }
            return val;
        }
        bool set_tcp_keepcnt(int val) {
            if(!ok()) {
                throw socket_error("socket::set_tcp_keepcnt(): socket not ready");
            }
            int maxpkt = val;
            return ::setsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPCNT, (char const *)&maxpkt, sizeof(int)) == 0;
        }
        int get_tcp_keepcnt() const {
            if(!ok()) {
                throw socket_error("socket::get_tcp_keepcnt(): socket not ready");
            }
            socklen_t len = sizeof(int);
            int val{};
            if(::getsockopt(sock_fd_, IPPROTO_TCP, TCP_KEEPCNT, (char *)&val, &len) != 0) {
                throw socket_error("socket::get_tcp_keepcnt(): failed to query option");
            }
            return val;
        }
        bool set_rcv_timeout(teal::timespec_wrapper const &to, teal::timespec_wrapper &orig_to) {
            bool result{false};
            if(sock_fd_ != -1) {
#if defined(PLATFORM_LINUX) || defined(PLATFOM_APPLE) || defined(PLATFORM_ANDROID)
                struct timeval org_tv{};
                socklen_t org_tv_len{sizeof(org_tv)};
                if(::getsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, &org_tv, &org_tv_len) == 0) {
                    orig_to = teal::timespec_wrapper{(long double)org_tv.tv_sec + (long double)org_tv.tv_usec / 1'000'000.0L};
                    struct timeval tv{(time_t)to.seconds(), (long)((to.fseconds() - to.seconds()) * 1000000)};
                    if(::setsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, (char const *)&tv, sizeof(tv)) == 0) {
                        result = true;
                    }
                }
#elif defined(PLATFOM_WINDOWS)
                DWORD timeout = to.seconds() * 1000;
                ::setsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
#endif
            }
            return result;
        }
        std::string peer_addr() const noexcept {
            std::string res{};
            if(sock_type_ == address_family::inet4) {
                res = ntop(common_addr_.sock_addr_.sin_addr);
            } else if(sock_type_ == address_family::inet6) {
                res = ntop(common_addr_.sock_addr6_.sin6_addr);
            }
            return res;
        }
        bool ok() const noexcept {
            return
                sock_fd_ >= 0
                &&
                (
                   sock_type_ == address_family::inet4
                   ||
                   sock_type_ == address_family::inet6
                   ||
                   sock_type_ == address_family::unix_socket
                )
            ;
        }
        operator bool() const noexcept {
            return ok();
        }
        bool still_engaged() const noexcept {
            if(ok()) {
                try {
                    return error_status() == 0;
                } catch (...) {
                }
            }
            return false;
        }

        int error_status() const {
            if(sock_fd_ >= 0) {
                int error_code{};
#ifdef PLATFORM_WINDOWS
                int error_code_size{sizeof(error_code)};
                int qres{::getsockopt(sock_fd_, SOL_SOCKET, SO_ERROR, (char *)&error_code, &error_code_size)};
#else
                socklen_t error_code_size{sizeof(error_code)};
                int qres{::getsockopt(sock_fd_, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size)};
#endif
                if(qres != 0) {
                    throw std::runtime_error{"error code request failed"};
                } else {
                    return error_code;
                }
            } else {
                throw std::runtime_error{"socket not ready"};
            }
        }

        int send_message(std::vector<std::uint8_t> const &data) {
            return send_message(data.data(), data.size());
        }

        int send_message(const void *data, std::uint32_t data_size) {
            int res{};
            if(data && data_size > 0) {
                std::uint32_t net_size = teal::bit_util::hnswap<std::uint32_t>{data_size}.val;
                std::uint32_t total_size = sizeof(std::uint32_t);
                std::uint32_t total_sent = 0;
                const char *curr_buff = reinterpret_cast<const char *>(&net_size);
                do {
                    int wrote_count = teal::net::socket::send(curr_buff + total_sent, total_size - total_sent);
                    if(wrote_count < 0) {
                        int e{static_cast<int>(teal::sys_util::last_error())};
                        while(e == EAGAIN) {
                            wrote_count = teal::net::socket::send(curr_buff + total_sent, total_size - total_sent);
                            if(wrote_count < 0) {
                                e = teal::sys_util::last_error();
                            } else {
                                e = 0;
                            }
                        }
                        if(e != 0) {
                            throw socket_error{teal::sys_util::error_str(teal::sys_util::last_error())};
                        }
                    }
                    total_sent += wrote_count;
                } while(total_sent != total_size);
                total_size = data_size;
                total_sent = 0;
                curr_buff = reinterpret_cast<const char *>(data);
                do {
                    int wrote_count = teal::net::socket::send(curr_buff + total_sent, total_size - total_sent);
                    if(wrote_count < 0) {
                        int e{static_cast<int>(sys_util::last_error())};
                        while(e == EAGAIN) {
                            wrote_count = teal::net::socket::send(curr_buff + total_sent, total_size - total_sent);
                            if(wrote_count < 0) {
                                e = sys_util::last_error();
                            } else {
                                e = 0;
                            }
                        }
                        if(e != 0) {
                            throw socket_error{teal::sys_util::error_str(teal::sys_util::last_error())};
                        }
                    }
                    total_sent += wrote_count;
                } while(total_sent != total_size);
                res = total_sent;
            }
            return res;
        }

#if 1
    std::vector<std::uint8_t> receive_message() {
        std::vector<std::uint8_t> result{};
        if(ok()) {
            std::vector<std::uint8_t> msg_size_vec{};
            for(msg_size_vec = socket::receive(4); msg_size_vec.size() < 4; ) {
                std::vector<std::uint8_t> msg_size_vec_part{socket::receive(4 - msg_size_vec.size())};
                if(msg_size_vec_part.size() > 0) {
                    msg_size_vec.insert(msg_size_vec.end(), msg_size_vec_part.begin(), msg_size_vec_part.end());
                }
            }
            if(msg_size_vec.size() == sizeof(std::uint32_t)) {
                std::uint32_t msg_size{teal::bit_util::from_net_bytes<std::uint32_t>(msg_size_vec.data())};
                std::uint32_t msg_size_host{teal::bit_util::hnswap<std::uint32_t>{msg_size}.val};
                for(result = teal::net::socket::receive(msg_size_host); result.size() < msg_size_host; ) {
                    std::vector<std::uint8_t> data_partial_vec{teal::net::socket::receive(msg_size_host - result.size())};
                    if(data_partial_vec.size() > 0) {
                        result.insert(result.end(), data_partial_vec.begin(), data_partial_vec.end());
                    }
                }
            }
        } else {
            throw std::runtime_error{"receive_message(): invalid socket state"};
        }
        return result;
    }
#endif

    protected:
#ifndef PLATFORM_WINDOWS
        bool set_fd_flag(unsigned int flag) {
            if(!ok()) {
                throw socket_error("socket::set_fd_flag(): socket not ready");
            }
            int flags, s;
            flags = ::fcntl(sock_fd_, F_GETFL, 0);
            if (flags == -1) {
                return false;
            }
            flags |= flag;
            s = ::fcntl (sock_fd_, F_SETFL, flags);
            if (s == -1) {
                return false;
            }
            return true;
        }

        bool reset_fd_flag(unsigned int flag) {
            if(!ok()) {
                throw socket_error("socket::reset_fd_flag(): socket not ready");
            }
            int flags, s;
            flags = fcntl(sock_fd_, F_GETFL, 0);
            if (flags == -1) {
                return false;
            }
            flags &= ~flag;
            s = ::fcntl(sock_fd_, F_SETFL, flags);
            if (s == -1) {
                return false;
            }
            return true;
        }
#endif

    protected:
        int sock_fd_{-1};
        address_family sock_type_{address_family::unspecified};
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        std::string unix_server_socket_path_{};
#endif
        union {
            struct sockaddr_in sock_addr_;
            struct sockaddr_in6 sock_addr6_;
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
            struct sockaddr_un sock_addr_unix_;
#endif
        } common_addr_;
    };

}
