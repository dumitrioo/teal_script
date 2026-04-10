#pragma once

#include "../commondefs.hpp"
#include "../timespec_wrapper.hpp"
#include "../serialization.hpp"
#include "../terminable.hpp"
#include "../crypto/gamma.hpp"
#include "../sequence_generator.hpp"
#include "net_utils.hpp"

namespace teal::net {

    static std::uint32_t constexpr UDP_ONESHOT_SIZE_MAX = 1280;
    static std::size_t constexpr UDP_BUF_SIZE{2048};
    static std::int8_t constexpr UDP_TRANSPORT_OP_CONNECT = 0;
    static std::int8_t constexpr UDP_TRANSPORT_OP_CONNECT_ACK = 1;
    static std::int8_t constexpr UDP_TRANSPORT_OP_DISCONNECT = 2;
    static std::int8_t constexpr UDP_TRANSPORT_OP_DATA = 3;
    static std::int8_t constexpr UDP_TRANSPORT_OP_LONGDATA = 4;
    static std::int8_t constexpr UDP_TRANSPORT_OP_SERVICE_DATA = 5;
    static std::int8_t constexpr UDP_TRANSPORT_OP_ENCRYPTED_DATA = 6;

    namespace helpers {

        static bool set_rcv_timeout(int sockfd, long double to, long double *orig_to = nullptr) {
            bool result{false};
            if(sockfd != -1) {
#if defined(PLATFORM_LINUX) || defined(PLATFOM_APPLE) || defined(PLATFORM_ANDROID)
                struct timeval org_tv{};
                socklen_t org_tv_len{sizeof(org_tv)};
                if(::getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &org_tv, &org_tv_len) == 0) {
                    if(orig_to) {
                        *orig_to = (long double)org_tv.tv_sec + (long double)org_tv.tv_usec / 1'000'000.0L;
                    }
                    struct timeval tv{(time_t)to, (long)((to - (long long)to) * 1000000)};
                    if(::setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char const *)&tv, sizeof(tv)) == 0) {
                        result = true;
                    }
                }
#elif defined(PLATFOM_WINDOWS)
                DWORD timeout = to.fseconds() * 1000;
                ::setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
#endif
            }
            return result;
        }

#ifndef PLATFORM_WINDOWS
        static bool set_fd_flag(int sock_fd, unsigned int flag) {
            int flags, s;
            flags = ::fcntl(sock_fd, F_GETFL, 0);
            if (flags == -1) {
                return false;
            }
            flags |= flag;
            s = ::fcntl (sock_fd, F_SETFL, flags);
            if (s == -1) {
                return false;
            }
            return true;
        }

#if 0
        static bool reset_fd_flag(int sock_fd, unsigned int flag) {
            int flags, s;
            flags = fcntl(sock_fd, F_GETFL, 0);
            if (flags == -1) {
                return false;
            }
            flags &= ~flag;
            s = ::fcntl(sock_fd, F_SETFL, flags);
            if (s == -1) {
                return false;
            }
            return true;
        }
#endif
#endif

        static bool make_nonblocking(int sock_fd) {
#ifdef PLATFORM_WINDOWS
            u_long iMode = 1;
            return ioctlsocket(sock_fd, FIONBIO, &iMode) == NO_ERROR;
#else
            return set_fd_flag(sock_fd, O_NONBLOCK);
#endif
        }

    }

}
