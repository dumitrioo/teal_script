#pragma once

#include "../commondefs.hpp"
#include "../str_util.hpp"
#include "../bit_util.hpp"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <sys/un.h>
#include <sys/uio.h>
#endif

namespace scfx::net {

    enum class address_family: int {
        unspecified = AF_UNSPEC,
        inet4 = AF_INET,
        inet6 = AF_INET6,
#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
        unix_socket = AF_UNIX,
#endif
    };

    enum class sock_type: int {
        unspecified = 0,
        stream = SOCK_STREAM,
        dgram = SOCK_DGRAM,
        raw = SOCK_RAW,
        rdm = SOCK_RDM,
        seqpacket = SOCK_SEQPACKET,
        dccp = SOCK_DCCP,
        packet = SOCK_PACKET,
        cloexec = SOCK_CLOEXEC,
        nonblock = SOCK_NONBLOCK,
    };

    static bool is_valid_ipv4_adr_string(std::string addr_str) {
        bool result{false};
        unsigned char buf[sizeof(struct in6_addr)];
        char str[INET6_ADDRSTRLEN];
        int s{::inet_pton(AF_INET, addr_str.c_str(), buf)};
        if (s > 0 && inet_ntop(AF_INET, buf, str, INET6_ADDRSTRLEN) != NULL) {
            result = true;
        }
        return result;
    }

    static bool is_valid_ipv6_adr_string(std::string addr_str) {
        bool result{false};
        unsigned char buf[sizeof(struct in6_addr)];
        char str[INET6_ADDRSTRLEN];
        int s{::inet_pton(AF_INET6, addr_str.c_str(), buf)};
        if (s > 0 && inet_ntop(AF_INET6, buf, str, INET6_ADDRSTRLEN) != NULL) {
            result = true;
        }
        return result;
    }


    static std::string ntop(in6_addr const &a) {
        std::string res{};
        std::vector<char> buff{}; buff.resize(512);
        auto ret{inet_ntop(AF_INET6, &a, &buff[0], buff.size())};
        if(ret) {
            res = ret;
        }
        return res;
    }

    static std::string ntop(in_addr const &a) {
        std::string res{};
        std::vector<char> buff{}; buff.resize(512);
        auto ret{inet_ntop(AF_INET, &a, &buff[0], buff.size())};
        if(ret) {
            res = ret;
        }
        return res;
    }

    static std::string ntop(in_addr_t i) {
        std::string res{};
        in_addr a{};
        a.s_addr = scfx::bit_util::swap_on_le<in_addr_t>{i}.val;
        std::vector<char> buff{}; buff.resize(512);
        auto ret{inet_ntop(AF_INET, &a, &buff[0], buff.size())};
        if(ret) {
            res = ret;
        }
        return res;
    }

    DEFINE_RUNTIME_ERROR_CLASS(addr_format_error)

    static in6_addr pton6(std::string const &a) {
        in6_addr res{};
        if(!inet_pton(AF_INET6, a.data(), &res)) {
            throw addr_format_error{"error converting address"};
        }
        return res;
    }

    static in_addr pton(std::string const &a) {
        in_addr res{};
        if(!inet_pton(AF_INET, a.data(), &res)) {
            throw addr_format_error{"error converting address"};
        }
        return res;
    }

    static std::string ipv4_to_hex(std::string const &addr) {
        in_addr an{scfx::net::pton(addr)};
        std::string res{scfx::str_util::utoa(scfx::bit_util::swap_on_le<std::uint32_t>{an.s_addr}.val, 16)};
        while(res.size() < 8) {
            res = std::string{"0"} + res;
        }
        return res;
    }

    static std::string ipv6_to_hex(std::string const &addr) {
        std::string res{};
        in6_addr an{scfx::net::pton6(addr)};
        for(int i{0}; i < 16; ++i) {
            std::string ns{scfx::str_util::utoa(((std::uint8_t const *)&an)[i], 16)};
            while(ns.size() < 2) {
                ns = std::string{"0"} + ns;
            }
            res += ns;
        }
        return res;
    }

    DEFINE_RUNTIME_ERROR_CLASS(resolve_error)

    static in_addr resolve(const std::string &name) {
        in_addr res;
        struct in_addr *addr_ptr;
        struct hostent *hostPtr;
        hostPtr = ::gethostbyname(name.c_str());
        if(hostPtr == NULL) {
            throw resolve_error{"error calling gethostbyname()"};
        }
        addr_ptr = (struct in_addr *)*hostPtr->h_addr_list;
        if(addr_ptr) {
            std::memcpy(&res, addr_ptr, hostPtr->h_length);
        } else {
            throw resolve_error{"not resolved"};
        }
        return res;
    }

    static in6_addr resolve6(const std::string &name) {
        in6_addr res;
        struct in6_addr *addr_ptr;
        struct hostent *hostPtr;
        hostPtr = ::gethostbyname2(name.c_str(), AF_INET6);
        if(hostPtr == NULL) {
            throw resolve_error{"error calling gethostbyname()"};
        }
        addr_ptr = (struct in6_addr *)*hostPtr->h_addr_list;
        if(addr_ptr) {
            std::memcpy(&res, addr_ptr, hostPtr->h_length);
        } else {
            throw resolve_error{"not resolved"};
        }
        return res;
    }

}
