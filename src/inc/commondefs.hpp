#pragma once
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <climits>
#include <cstdio>
#include <ctime>
#include <new>
#include <cstdarg>
#include <ios>
#include <iomanip>
#include <fstream>
#include <csignal>
#include <functional>
#include <future>
#include <string>
#include <cstring>
#include <cwctype>
#include <vector>
#include <deque>
#include <queue>
#include <list>
#include <map>
#include <stack>
#include <limits>
#include <set>
#include <bitset>
#include <typeinfo>
#include <memory>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <iterator>
#include <complex>
#include <type_traits>
#include <cerrno>
#if (__cplusplus >= 201103L)
#include <valarray>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <array>
#include <locale>
#include <cassert>
#include <cstdint>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include <regex>
#include <ratio>
#endif
#if (__cplusplus >= 201400L)
#include <shared_mutex>
#include <chrono>
#endif
#if (__cplusplus >= 201703L)
#include <any>
#include <variant>
#include <optional>
#include <string_view>
#include <filesystem>
#endif
#if (__cplusplus >= 202002L)
#include <semaphore>
#include <ranges>
#include <numbers>
#include <compare>
#include <concepts>
#include <coroutine>
#include <span>
#include <format>
#endif
#if (__cplusplus >= 202302L)
#include <print>
#include <stacktrace>
#endif
#include <cstdio>
#include <cmath>
#include <streambuf>
#include <istream>
#include <sstream>
#include <ostream>
#include <exception>
#include <stdexcept>
#include <cassert>

#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__ppc64__)
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#if defined(_WIN64)
    #define PLATFORM_WINDOWS
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #ifndef ENVIRONMENT64
        #define ENVIRONMENT64
    #endif
#elif defined(_WIN32)
    #define PLATFORM_WINDOWS
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #ifndef ENVIRONMENT32
        #define ENVIRONMENT32
    #endif
#elif defined(__APPLE__)
    #define PLATFORM_APPLE
    #include <fcntl.h>
    #include <unistd.h>
    #include <stdlib.h>
    #if TARGET_OS_IPHONE
    #elif TARGET_IPHONE_SIMULATOR
    #elif TARGET_OS_MAC
    #else
    #endif
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID
    #if !defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 500
    #define _XOPEN_SOURCE 500
    #endif
    #include <signal.h>
    #include <time.h>
    #include <sys/wait.h>
    #include <netinet/in.h>
    #include <netinet/ip6.h>
    #include <sys/mman.h>
    #include <dlfcn.h>
    #include <sys/resource.h>
#define _POSIX1_SOURCE 2
    #include <unistd.h>
    #include <semaphore.h>
    #include <assert.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <stdint.h>
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <ftw.h>
    #include <sys/poll.h>
    #include <pthread.h>
    #include <arpa/inet.h>
    #include <sys/utsname.h>
    #include <netinet/ip.h>
    #include <netinet/ip_icmp.h>
    #include <sys/random.h>
    #include <sched.h>
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #if !defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 500
    #define _XOPEN_SOURCE 500
    #endif
    #include <signal.h>
    #include <time.h>
    #include <sys/wait.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <getopt.h>
    #include <netinet/in.h>
    #include <netinet/ip6.h>
    #include <sys/mman.h>
    #include <dlfcn.h>
    #include <sys/resource.h>
#define _POSIX1_SOURCE 2
    #include <unistd.h>
    #include <semaphore.h>
    #include <assert.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <stdint.h>
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <ftw.h>
    #include <sys/poll.h>
    #include <pthread.h>
    #include <arpa/inet.h>
    #include <sys/utsname.h>
    #include <netinet/ip.h>
    #include <netinet/ip_icmp.h>
    #include <sys/random.h>
    #include <sched.h>
#elif defined(__unix) // all unices not caught above
    #define PLATFORM_UNIXISH
#elif defined(__posix)
    #define PLATFORM_POSIX
#endif

#if defined(_MSC_VER)
    #define COMPILER_MSC
#elif defined(__GNUC__)
    #define COMPILER_GCC
#elif defined(__clang__)
    #define COMPILER_CLANG
#elif defined(__EMSCRIPTEN__)
    #define COMPILER_EMSCRIPTEN
#elif defined(__MINGW64__)
    #ifndef ENVIRONMENT64
        #define ENVIRONMENT64
    #endif
    #define COMPILER_MINGW
#elif defined(__MINGW32__)
    #ifndef ENVIRONMENT32
        #define ENVIRONMENT32
    #endif
    #define COMPILER_MINGW
#endif


#define DEFINE_EXCEPTION_CLASS_FROM(CLASS_NAME, FROM_CLASS) class CLASS_NAME: public FROM_CLASS { \
    public: \
            CLASS_NAME(const std::string &msg): FROM_CLASS(msg) { \
        } \
    };

#define DEFINE_RUNTIME_ERROR_CLASS(CLASS_NAME) \
    class CLASS_NAME: public std::runtime_error { \
    public: \
      CLASS_NAME(const std::string &arg): std::runtime_error{arg} {} \
    };

#define DEFINE_RUNTIME_ERROR_CLASS_MSG(CLASS_NAME, MESSAGE) \
    class CLASS_NAME: public std::runtime_error { \
    public: \
      CLASS_NAME(): std::runtime_error{MESSAGE} {} \
    };


#define INLINE_GETTER_SETTER(TYPE, NAME, FIELD) \
TYPE const &NAME() const { \
        return FIELD; \
} \
    void set_ ## NAME(TYPE const &f) { \
        FIELD = f; \
}

namespace scfx {

    using int64_t = std::int64_t;
    using uint64_t = std::uint64_t;
    using int32_t = std::int32_t;
    using uint32_t = std::uint32_t;
    using int16_t = std::int16_t;
    using uint16_t = std::uint16_t;
    using int8_t = std::int8_t;
    using uint8_t = std::uint8_t;

    using off64_t = std::int64_t;
    using pos64_t = std::uint64_t;
    using off_t = off64_t;
    using pos_t = pos64_t;

    template <std::size_t byte_size>
    using uintn_t =
        typename std::conditional<byte_size == 1, std::uint8_t,
            typename std::conditional<byte_size == 2, std::uint16_t,
                typename std::conditional<byte_size == 3 || byte_size == 4, std::uint32_t,
                    std::uint64_t
                >::type
            >::type
        >::type;

#ifdef PLATFORM_WINDOWS
#if defined(ENVIRONMENT64)
    using ssize_t = std::int64_t ;
#elif defined(ENVIRONMENT32)
    using ssize_t = std::int32_t;
#endif
#endif

    using bytevec = std::vector<std::uint8_t>;

    template<typename FUNCTOR>
    class shut_on_destroy final {
    public:
        shut_on_destroy(FUNCTOR const &d): dref_{d} {}
        shut_on_destroy(FUNCTOR &&d): dref_{std::move(d)} {}
        shut_on_destroy(shut_on_destroy const &) = delete;
        shut_on_destroy &operator=(shut_on_destroy const &) = delete;
        shut_on_destroy(shut_on_destroy &&) = default;
        shut_on_destroy &operator=(shut_on_destroy &&) = default;
        ~shut_on_destroy() { dref_(); }

    private:
        FUNCTOR dref_;
    };

}
