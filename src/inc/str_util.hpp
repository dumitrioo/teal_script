#pragma once

#include "commondefs.hpp"
#ifdef PLATFORM_WINDOWS
#include <wctype.h>
#include <wchar.h>
#endif
#include "emhash/hash_table8.hpp"
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
#include "unicode_operations.hpp"
#endif

namespace scfx::str_util {

    template<typename T, std::size_t N, typename U>
#if (__cplusplus >= 202000L)
        requires(N < 256)
#endif
    static constexpr std::array<U, 256> create_reverse_table(
        std::array<T, N> const &direct_table, U blank_value
        ) {
        std::array<U, 256> res{};
        for(std::size_t i{0}; i < 256; ++i) { res[i] = blank_value; }
        for(std::size_t i{0}; i < N; ++i) { res[direct_table[i]] = i; }
        return res;
    }

    inline long double atof(std::string const &v) {
        return std::atof(v.c_str());
    }

    template<typename T>
    inline std::string ftoa(T v, std::size_t prec = 6) {
        std::ostringstream s;
        s << std::setprecision(prec) << std::fixed << v;
        std::string res{s.str()};
        auto pos{res.find('.')};
        if(pos == std::string::npos) { pos = res.find(','); }
        if(pos != std::string::npos) {
            auto i{res.size() - 1};
            while(i > pos) {
                if(res[i] == '0') {
                    --i;
                } else {
                    break;
                }
            }
            res = res.substr(0, i + 1);
            if(res.size() && (res[res.size() - 1] == '.' || res[res.size() - 1] == ',')) {
                res = res.substr(0, res.size() - 1);
            }
        }
        return res;
    }

    template<typename S_T>
    S_T utoa(std::uint64_t value, int radix = 10, std::int64_t width = 0, bool upcase = false) {
        static char const *digits[2] = {
            "0123456789abcdefghijklmnopqrstuvwxyz",
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        };
        if((radix < 2) || (radix > 36)) {
            throw std::runtime_error("invalid base system radix");
        }
        std::array<char, 128> char_stack{};
        std::int64_t char_stack_size{0};
        if(value == 0) {
            char_stack[char_stack_size++] = '0';
        } else {
            for(; value != 0; value /= radix) {
                std::uint64_t k{value % radix};
                char_stack[char_stack_size++] = digits[upcase ? 1 : 0][k];
            }
        }
        S_T res{};
        res.reserve(char_stack_size + 1);
        std::int64_t num_prepends{width - char_stack_size};
        if(width > 0 && num_prepends > 0) {
            res.reserve(num_prepends + char_stack_size);
            for(int64_t i{}; i < num_prepends; ++i) {
                res.push_back('0');
            }
        } else {
            res.reserve(char_stack_size);
        }
        for(std::int64_t i{char_stack_size - 1}; i >= 0; --i) {
            res.push_back(char_stack[i]);
        }
        return res;
    }

    template<typename S_T>
    S_T itoa(std::int64_t value, int radix = 10, std::int64_t width = 0, bool upcase = false) {
        static const char *digits[2] = {
            "0123456789abcdefghijklmnopqrstuvwxyz",
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        };
        if((radix < 2) || (radix > 36)) {
            throw std::runtime_error("invalid base system radix");
        }
        std::array<char, 128> char_stack{};
        std::int64_t char_stack_size{0};
        bool sign{value < 0};
        if(value == 0) {
            char_stack[char_stack_size++] = '0';
        } else {
            for(std::int64_t val{sign ? -value : value}; val != 0; val /= radix) {
                std::int64_t k = val % radix;
                char_stack[char_stack_size++] = digits[upcase ? 1 : 0][k];
            }
        }
        S_T res{};
        std::int64_t num_prepends{width - char_stack_size - (sign ? 1 : 0)};
        if(width > 0 && num_prepends > 0) {
            res.reserve(num_prepends + char_stack_size + 1);
            if(sign) {
                res.push_back('-');
            }
            for(int64_t i{}; i < num_prepends; ++i) {
                res.push_back('0');
            }
        } else {
            res.reserve(char_stack_size + 1);
            if(sign) {
                res.push_back('-');
            }
        }
        for(std::int64_t i{char_stack_size - 1}; i >= 0; --i) {
            res.push_back(char_stack[i]);
        }
        return res;
    }

    template<typename S_T>
    std::int64_t atoi(const S_T &a, int radix = 10, bool enable_separators = false) {
        static std::array<int, 256> constexpr digits{
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -2,-1,-1,-1,-1,-1,-1,-2,-1,-1,-1,-1,-2,-1,-1,-1,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
            -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
            25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
            -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
            25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        };
        size_t str_size{a.size()};
        if(radix < 2 || radix > 36) {
            throw std::runtime_error{"invalid radix"};
        }
        std::size_t start_index{0};
        bool negative{false};
        if(a[0] == '-') {
            negative = true;
            start_index = 1;
        } else if(a[0] == '+') {
            negative = false;
            start_index = 1;
        }
        std::int64_t result{0};
        int s{0};
        int nn{0};
        for(std::size_t i{start_index}; i < str_size; ++i) {
            uint8_t c{static_cast<uint8_t>(a[i])};
            std::int64_t d{static_cast<std::int64_t>(digits[c])};
            if(enable_separators && d == -2) {
                if(s <= 0) {
                    throw std::runtime_error{"invalid number"};
                }
                s = -1;
                continue;
            }
            if(d >= radix || d < 0) {
                throw std::runtime_error{"invalid number"};
            }
            result *= static_cast<std::int64_t>(radix);
            result += d;
            ++nn;
            s = 1;
        }
        if(nn <= 0) {
            throw std::runtime_error{"string does not contain a number"};
        }
        if(s <= 0) {
            throw std::runtime_error{"wrong number format"};
        }
        return negative ? -result : result;
    }


    static inline std::int64_t atoi(char const *a, int radix = 10) {
        return atoi<std::string>(a, radix);
    }

    static inline std::int64_t atoi(wchar_t const *a, int radix = 10) {
        return atoi<std::wstring>(a, radix);
    }

    template<typename S_T>
    std::uint64_t atoui(S_T const &a, int radix = 10, bool enable_separators = false) {
        static std::array<int, 256> constexpr digits{
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -2,-1,-1,-1,-1,-1,-1,-2,-1,-1,-1,-1,-2,-1,-1,-1,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
            -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
            25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
            -1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
            25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        };
        size_t str_size{a.size()};
        if(str_size == 0) {
            throw std::runtime_error{"empty string passed"};
        }
        if(radix < 2 || radix > 36) {
            throw std::runtime_error{"invalid radix"};
        }
        std::size_t start_index{0};
        bool negative{false};
        if(a[0] == '-') {
            negative = true;
            start_index = 1;
        } else if(a[0] == '+') {
            negative = false;
            start_index = 1;
        }
        std::uint64_t result{0};
        int s{0};
        int nn{0};
        for(std::size_t i{start_index}; i < str_size; ++i) {
            uint8_t c{static_cast<uint8_t>(a[i])};
            std::int64_t d{static_cast<std::int64_t>(digits[c])};
            if(enable_separators && d == -2) {
                if(s <= 0) {
                    throw std::runtime_error{"wrong number format"};
                }
                s = -1;
                continue;
            }
            if(d >= radix || d < 0) {
                throw std::runtime_error{"invalid number"};
            }
            result *= static_cast<std::uint64_t>(radix);
            result += static_cast<std::uint64_t>(d);
            ++nn;
            s = 1;
        }
        if(nn <= 0) {
            throw std::runtime_error{"string does not contain a number"};
        }
        if(s <= 0) {
            throw std::runtime_error{"wrong number format"};
        }
        return negative ? -result : result;
    }

    template<typename string_t>
    string_t replace_substring(string_t const &where, string_t const &what, string_t const &to) {
        string_t res{where};
        std::size_t pos{0};
        while(true) {
            pos = res.find(what, pos);
            if(pos == string_t::npos) {
                break;
            }
            res.replace(pos, what.size(), to);
            pos += to.size();
        }
        return res;
    }

    template<typename STR_T>
    bool string_starts_from(const STR_T &str, const STR_T &from) {
        return (str.empty() && from.empty())
                ||
               (str.size() >= from.size()
                &&
                str.substr(0, from.size()) == from);
    }

    static int towupper(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::toupper(c);
#else
        return std::towupper(c);
#endif
    }

    static int towlower(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::tolower(c);
#else
        return std::towlower(c);
#endif
    }

    static int iswalpha(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::isalpha(c);
#else
        return std::iswalpha(c);
#endif
    }

    static int iswdigit(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::isdigit(c);
#else
        return std::iswdigit(c);
#endif
    }

    static int iswalnum(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::isalnum(c);
#else
        return std::iswalnum(c);
#endif
    }

    static int iswlower(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::islower(c);
#else
        return std::iswlower(c);
#endif
    }

    static int iswupper(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::isupper(c);
#else
        return std::iswupper(c);
#endif
    }

    static int iswprint(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::isprint(c);
#else
        return std::iswprint(c);
#endif
    }

    static int iswgraph(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::isgraph(c);
#else
        return std::iswgraph(c);
#endif
    }

    static int iswascii_(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::isascii(c);
#else
        return ::isascii(c);
#endif
    }

    static int iswcntrl(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::iscntrl(c);
#else
        return std::iswcntrl(c);
#endif
    }

    static int iswpunct(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::ispunct(c);
#else
        return std::iswpunct(c);
#endif
    }

    static int iswspace(int c) {
#ifdef STR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS
        return unicode_operations::isspace(c);
#else
        return std::iswspace(c);
#endif
    }

    static int toupper(int c) {
        return std::toupper(c);
    }

    static int tolower(int c) {
        return std::tolower(c);
    }

    static int isalpha(int c) {
        return std::isalpha(c);
    }

    static int isdigit(int c) {
        return std::isdigit(c);
    }

    static int isalnum(int c) {
        return std::isalnum(c);
    }

    static int islower(int c) {
        return std::islower(c);
    }

    static int isupper(int c) {
        return std::isupper(c);
    }

    static int isprint(int c) {
        return std::isprint(c);
    }

    static int isgraph(int c) {
        return std::isgraph(c);
    }

    static int isascii_(int c) {
        return ::isascii(c);
    }

    static int iscntrl(int c) {
        return ::iscntrl(c);
    }

    static int ispunct(int c) {
        return ::ispunct(c);
    }

    static int isspace(int c) {
        return ::isspace(c);
    }

    static int ishex(int c) {
        static const std::uint8_t digits[128] = {
            36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,
            36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,
            36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,
             0, 1, 2, 3, 4, 5, 6, 7, 8, 9,36,36,36,36,36,36,
            36,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
            25,26,27,28,29,30,31,32,33,34,35,36,36,36,36,36,
            36,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
            25,26,27,28,29,30,31,32,33,34,35,36,36,36,36,36,
        };
        return static_cast<uint32_t>(c) < 128 && digits[c] < 16;
    }

    std::int64_t utf8_to_ucs(void const *ut8, int *increment) {
        const unsigned char *utf8{reinterpret_cast<const unsigned char *>(ut8)};

        if(utf8[0] < 0x80) {
            if(increment) *increment = 1;
            return utf8[0] & 0xff;
        }

        if(((utf8[0] & 0xe0) == 0xc0)) {
            if(increment) *increment = 2;
            return (((std::uint64_t)utf8[0] & 0x1f) << 6) |
                   (((std::uint64_t)utf8[1] & 0x3f));
        }

        if(((utf8[0] & 0xf0) == 0xe0)) {
            if(increment) *increment = 3;
            return (((std::uint64_t)utf8[0] & 0x0f) << 12) |
                   (((std::uint64_t)utf8[1] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[2] & 0x3f));
        }

        if(((utf8[0] & 0xf8) == 0xf0)) {
            if(increment) *increment = 4;
            return (((std::uint64_t)utf8[0] & 0x07) << 18) |
                   (((std::uint64_t)utf8[1] & 0x3f) << 12) |
                   (((std::uint64_t)utf8[2] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[3] & 0x3f));
        }

        if(((utf8[0] & 0xfc) == 0xf8)) {
            if(increment) *increment = 5;
            return (((std::uint64_t)utf8[0] & 0x03) << 24) |
                   (((std::uint64_t)utf8[1] & 0x3f) << 18) |
                   (((std::uint64_t)utf8[2] & 0x3f) << 12) |
                   (((std::uint64_t)utf8[3] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[4] & 0x3f));
        }

        if(((utf8[0] & 0xfe) == 0xfc)) {
            if(increment) *increment = 6;
            return (((std::uint64_t)utf8[0] & 0x01) << 30) |
                   (((std::uint64_t)utf8[1] & 0x3f) << 24) |
                   (((std::uint64_t)utf8[2] & 0x3f) << 18) |
                   (((std::uint64_t)utf8[3] & 0x3f) << 12) |
                   (((std::uint64_t)utf8[4] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[5] & 0x3f));
        }

        if(utf8[0] == 0xfe) {
            if(increment) *increment = 7;
            return (((std::uint64_t)utf8[1] & 0x3f) << 30) |
                   (((std::uint64_t)utf8[2] & 0x3f) << 24) |
                   (((std::uint64_t)utf8[3] & 0x3f) << 18) |
                   (((std::uint64_t)utf8[4] & 0x3f) << 12) |
                   (((std::uint64_t)utf8[5] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[6] & 0x3f));
        }

        if(utf8[0] == 0xff && utf8[1] == 0x80) {
            if(increment) *increment = 13;
            return (std::uint64_t)(((std::uint64_t)(utf8[2]  & 0x3f) << 60)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[3]  & 0x3f) << 54)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[4]  & 0x3f) << 48)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[5]  & 0x3f) << 42)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[6]  & 0x3f) << 36)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[7]  & 0x3f) << 30)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[8]  & 0x3f) << 24)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[9]  & 0x3f) << 18)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[10] & 0x3f) << 12)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[11] & 0x3f) <<  6)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[12] & 0x3f) <<  0));
        }

        if(increment) *increment = 1;
        return utf8[0];
    }

    std::int64_t utf8_to_ucs_maxlen(void const *ut8, std::int64_t max_len, int *increment) {
        std::uint8_t const *utf8{reinterpret_cast<std::uint8_t const *>(ut8)};

        if(max_len < 1) {
            if(increment) { *increment = 0; }
            return -1;
        }

        if(utf8[0] < 0x80) {
            if(increment) *increment = 1;
            return utf8[0] & 0xff;
        }

        if(max_len < 2) {
            if(increment) { *increment = 0; }
            return -1;
        }

        if(((utf8[0] & 0xe0) == 0xc0)) {
            if(increment) *increment = 2;
            return (((std::uint64_t)utf8[0] & 0x1f) << 6) |
                   (((std::uint64_t)utf8[1] & 0x3f));
        }

        if(max_len < 3) {
            if(increment) { *increment = 0; }
            return -1;
        }

        if(((utf8[0] & 0xf0) == 0xe0)) {
            if(increment) *increment = 3;
            return (((std::uint64_t)utf8[0] & 0x0f) << 12) |
                   (((std::uint64_t)utf8[1] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[2] & 0x3f));
        }

        if(max_len < 4) {
            if(increment) { *increment = 0; }
            return -1;
        }

        if(((utf8[0] & 0xf8) == 0xf0)) {
            if(increment) *increment = 4;
            return (((std::uint64_t)utf8[0] & 0x07) << 18) |
                   (((std::uint64_t)utf8[1] & 0x3f) << 12) |
                   (((std::uint64_t)utf8[2] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[3] & 0x3f));
        }

        if(max_len < 5) {
            if(increment) { *increment = 0; }
            return -1;
        }

        if(((utf8[0] & 0xfc) == 0xf8)) {
            if(increment) *increment = 5;
            return (((std::uint64_t)utf8[0] & 0x03) << 24) |
                   (((std::uint64_t)utf8[1] & 0x3f) << 18) |
                   (((std::uint64_t)utf8[2] & 0x3f) << 12) |
                   (((std::uint64_t)utf8[3] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[4] & 0x3f));
        }

        if(max_len < 6) {
            if(increment) { *increment = 0; }
            return -1;
        }

        if(((utf8[0] & 0xfe) == 0xfc)) {
            if(increment) *increment = 6;
            return (((std::uint64_t)utf8[0] & 0x01) << 30) |
                   (((std::uint64_t)utf8[1] & 0x3f) << 24) |
                   (((std::uint64_t)utf8[2] & 0x3f) << 18) |
                   (((std::uint64_t)utf8[3] & 0x3f) << 12) |
                   (((std::uint64_t)utf8[4] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[5] & 0x3f));
        }

        if(max_len < 7) {
            if(increment) { *increment = 0; }
            return -1;
        }

        if(utf8[0] == 0xfe) {
            if(increment) *increment = 7;
            return (((std::uint64_t)utf8[1] & 0x3f) << 30) |
                   (((std::uint64_t)utf8[2] & 0x3f) << 24) |
                   (((std::uint64_t)utf8[3] & 0x3f) << 18) |
                   (((std::uint64_t)utf8[4] & 0x3f) << 12) |
                   (((std::uint64_t)utf8[5] & 0x3f) << 6) |
                   (((std::uint64_t)utf8[6] & 0x3f));
        }

        if(max_len < 13) {
            if(increment) { *increment = 0; }
            return -1;
        }

        if(utf8[0] == 0xff && utf8[1] == 0x80) {
            if(increment) *increment = 13;
            return (std::uint64_t)(((std::uint64_t)(utf8[2]  & 0x3f) << 60)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[3]  & 0x3f) << 54)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[4]  & 0x3f) << 48)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[5]  & 0x3f) << 42)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[6]  & 0x3f) << 36)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[7]  & 0x3f) << 30)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[8]  & 0x3f) << 24)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[9]  & 0x3f) << 18)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[10] & 0x3f) << 12)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[11] & 0x3f) <<  6)) |
                   (std::uint64_t)(((std::uint64_t)(utf8[12] & 0x3f) <<  0));
        }

        if(increment) {
            *increment = 1;
        }

        return utf8[0];
    }


    int ucs_to_utf8(std::uint64_t c, void *ut8) {
        std::uint8_t *utf8{(std::uint8_t *)ut8};
        if(c <= 0x7fULL) {
            utf8[0] = (std::uint8_t) c & 0x7f;
            utf8[1] = (std::uint8_t) 0;
            return 1;
        } else if(c <= 0x7'ffULL) {
            utf8[0] = (std::uint8_t)((c >> 6) & 0x1f) | 0xc0;
            utf8[1] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t) 0;
            return 2;
        } else if(c <= 0xff'ffULL) {
            utf8[0] = (std::uint8_t)((c >> 12) & 0xf) | 0xe0;
            utf8[1] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[3] = (std::uint8_t) 0;
            return 3;
        } else if(c <= 0x1f'ff'ffULL) {
            utf8[0] = (std::uint8_t)((c >> 18) & 7) | 0xf0;
            utf8[1] = (std::uint8_t)((c >> 12) & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
            utf8[3] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[4] = (std::uint8_t) 0;
            return 4;
        } else if(c <= 0x3'ff'ff'ffULL) {
            utf8[0] = (std::uint8_t)((c >> 24) & 3) | 0xf8;
            utf8[1] = (std::uint8_t)((c >> 18) & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t)((c >> 12) & 0x3f) | 0x80;
            utf8[3] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
            utf8[4] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[5] = (std::uint8_t) 0;
            return 5;
        } else if(c <= 0x7f'ff'ff'ffULL) {
            utf8[0] = (std::uint8_t)((c >> 30) & 1) | 0xfc;
            utf8[1] = (std::uint8_t)((c >> 24) & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t)((c >> 18) & 0x3f) | 0x80;
            utf8[3] = (std::uint8_t)((c >> 12) & 0x3f) | 0x80;
            utf8[4] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
            utf8[5] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[6] = (std::uint8_t) 0;
            return 6;
        } else if(c <= 0xf'ff'ff'ff'ffULL) {
            utf8[0] = 0xfe;
            utf8[1] = (std::uint8_t)(((c >> 30) & 0x3f) | 0x80);
            utf8[2] = (std::uint8_t)(((c >> 24) & 0x3f) | 0x80);
            utf8[3] = (std::uint8_t)(((c >> 18) & 0x3f) | 0x80);
            utf8[4] = (std::uint8_t)(((c >> 12) & 0x3f) | 0x80);
            utf8[5] = (std::uint8_t)(((c >> 6) & 0x3f) | 0x80);
            utf8[6] = (std::uint8_t)((c & 0x3f) | 0x80);
            utf8[7] = (std::uint8_t) 0;
            return 7;
        } else if(c <= 0x7f'ff'ff'ff'ff'ff'ff'ffULL) {
            utf8[0] = 0xff;
            utf8[1] = 0x80;
            utf8[2] = (std::uint8_t) (((c >> 60) & 0x3f) | 0x80);
            utf8[3] = (std::uint8_t) (((c >> 54) & 0x3f) | 0x80);
            utf8[4] = (std::uint8_t) (((c >> 48) & 0x3f) | 0x80);
            utf8[5] = (std::uint8_t) (((c >> 42) & 0x3f) | 0x80);
            utf8[6] = (std::uint8_t) (((c >> 36) & 0x3f) | 0x80);
            utf8[7] = (std::uint8_t) (((c >> 30) & 0x3f) | 0x80);
            utf8[8] = (std::uint8_t) (((c >> 24) & 0x3f) | 0x80);
            utf8[9] = (std::uint8_t) (((c >> 18) & 0x3f) | 0x80);
            utf8[10] = (std::uint8_t)(((c >> 12) & 0x3f) | 0x80);
            utf8[11] = (std::uint8_t)(((c >>  6) & 0x3f) | 0x80);
            utf8[12] = (std::uint8_t)(((c >>  0) & 0x3f) | 0x80);
            utf8[13] = (std::uint8_t) 0;
            return 13;
        }
        return 0;
    }

    std::string ucs_to_utf8(std::uint64_t c) {
        std::array<char, 32> buff{};
        int num{ucs_to_utf8(c, &buff[0])};
        return std::string{buff.begin(), buff.begin() + num};
    }

    int ucs_to_utf8_maxlen(std::uint64_t c, void *ut8, std::int64_t max_len) {
        std::uint8_t *utf8{(std::uint8_t *)ut8};
        if(c <= 0x7fULL) {
            if(max_len < 2) { return 0; }
            utf8[0] = (std::uint8_t) c & 0x7f;
            utf8[1] = (std::uint8_t) 0;
            return 1;
        } else if(c <= 0x7'ffULL) {
            if(max_len < 3) { return 0; }
            utf8[0] = (std::uint8_t)((c >> 6) & 0x1f) | 0xc0;
            utf8[1] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t) 0;
            return 2;
        } else if(c <= 0xff'ffULL) {
            if(max_len < 4) { return 0; }
            utf8[0] = (std::uint8_t)((c >> 12) & 0xf) | 0xe0;
            utf8[1] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[3] = (std::uint8_t) 0;
            return 3;
        } else if(c <= 0x1f'ff'ffULL) {
            if(max_len < 5) { return 0; }
            utf8[0] = (std::uint8_t)((c >> 18) & 7) | 0xf0;
            utf8[1] = (std::uint8_t)((c >> 12) & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
            utf8[3] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[4] = (std::uint8_t) 0;
            return 4;
        } else if(c <= 0x3'ff'ff'ffULL) {
            if(max_len < 6) { return 0; }
            utf8[0] = (std::uint8_t)((c >> 24) & 3) | 0xf8;
            utf8[1] = (std::uint8_t)((c >> 18) & 0x3f) | 0x80;
            utf8[2] = (std::uint8_t)((c >> 12) & 0x3f) | 0x80;
            utf8[3] = (std::uint8_t)((c >> 6) & 0x3f) | 0x80;
            utf8[4] = (std::uint8_t)(c & 0x3f) | 0x80;
            utf8[5] = (std::uint8_t) 0;
            return 5;
        } else if(c <= 0x7f'ff'ff'ffULL) {
            if(max_len < 7) { return 0; }
            utf8[0] = (std::uint8_t)(((c >> 30) & 1) | 0xfc);
            utf8[1] = (std::uint8_t)(((c >> 24) & 0x3f) | 0x80);
            utf8[2] = (std::uint8_t)(((c >> 18) & 0x3f) | 0x80);
            utf8[3] = (std::uint8_t)(((c >> 12) & 0x3f) | 0x80);
            utf8[4] = (std::uint8_t)(((c >> 6) & 0x3f) | 0x80);
            utf8[5] = (std::uint8_t)((c & 0x3f) | 0x80);
            utf8[6] = (std::uint8_t) 0;
            return 6;
        } else if(c <= 0xf'ff'ff'ff'ffULL) {
            if(max_len < 8) { return 0; }
            utf8[0] = 0xfe;
            utf8[1] = (std::uint8_t)(((c >> 30) & 0x3f) | 0x80);
            utf8[2] = (std::uint8_t)(((c >> 24) & 0x3f) | 0x80);
            utf8[3] = (std::uint8_t)(((c >> 18) & 0x3f) | 0x80);
            utf8[4] = (std::uint8_t)(((c >> 12) & 0x3f) | 0x80);
            utf8[5] = (std::uint8_t)(((c >> 6) & 0x3f) | 0x80);
            utf8[6] = (std::uint8_t)((c & 0x3f) | 0x80);
            utf8[7] = (std::uint8_t) 0;
            return 7;
        } else if(c <= 0x7f'ff'ff'ff'ff'ff'ff'ffULL) {
            if(max_len < 14) { return 0; }
            utf8[0] = 0xff;
            utf8[1] = 0x80;
            utf8[2] =  static_cast<std::uint8_t>(((c >> 60) & 0x0f) | 0x80);
            utf8[3] =  static_cast<std::uint8_t>(((c >> 54) & 0x3f) | 0x80);
            utf8[4] =  static_cast<std::uint8_t>(((c >> 48) & 0x3f) | 0x80);
            utf8[5] =  static_cast<std::uint8_t>(((c >> 42) & 0x3f) | 0x80);
            utf8[6] =  static_cast<std::uint8_t>(((c >> 36) & 0x3f) | 0x80);
            utf8[7] =  static_cast<std::uint8_t>(((c >> 30) & 0x3f) | 0x80);
            utf8[8] =  static_cast<std::uint8_t>(((c >> 24) & 0x3f) | 0x80);
            utf8[9] =  static_cast<std::uint8_t>(((c >> 18) & 0x3f) | 0x80);
            utf8[10] = static_cast<std::uint8_t>(((c >> 12) & 0x3f) | 0x80);
            utf8[11] = static_cast<std::uint8_t>(((c >>  6) & 0x3f) | 0x80);
            utf8[12] = static_cast<std::uint8_t>(((c >>  0) & 0x3f) | 0x80);
            utf8[13] = static_cast<std::uint8_t>(0);
            return 13;
        }
        return 0;
    }

    template<typename STR_T = std::wstring>
    std::string to_utf8(const STR_T &str) {
        std::string result{};
        result.reserve(str.size());
        for(auto &&wc: str) {
            char buf[32];
            int s = ucs_to_utf8(wc, buf);
            for(int j = 0; j < s; j++) {
                result += buf[j];
            }
        }
        return result;
    }

    template<typename char_type = wchar_t>
    std::string to_utf8(char_type const *str) {
        std::string result{};
        for(const char_type *wi{str}; wi && *wi; ++wi) {
            char buf[32];
            int s{ucs_to_utf8(*wi, buf)};
            for(int j = 0; j < s; j++) {
                result += buf[j];
            }
        }
        return std::string(result.begin(), result.end());
    }

    template<typename STR_T = std::wstring>
    void to_utf8(const STR_T &str, std::string &result) {
        result.reserve(str.size());
        for(auto &&wc: str) {
            char buf[32];
            int s{ucs_to_utf8(wc, buf)};
            for(int j = 0; j < s; j++) {
                result += buf[j];
            }
        }
    }

    template<typename char_type = wchar_t>
    void to_utf8(const char_type *str, std::string &result) {
        for(const char_type *wi{str}; wi && *wi; ++wi) {
            char buf[32];
            int s{ucs_to_utf8(*wi, buf)};
            for(int j = 0; j < s; j++) {
                result += buf[j];
            }
        }
    }

    template<typename STR_T = std::wstring>
    STR_T from_utf8(const std::string &s) {
        STR_T result{};
        result.reserve(s.size());
        int increment{};
        wchar_t c{};
        std::int64_t len{(std::int64_t)s.size()};
        for(
            char const *utf8{s.data()};
            len > 0 && ((c = utf8_to_ucs_maxlen(utf8, len, &increment)) >= 0);
            utf8 += increment, len -= increment
        ) {
            result += static_cast<typename STR_T::value_type>(c);
        }
        return result;
    }

    template<typename STR_T = std::wstring>
    STR_T from_utf8(const std::string_view &s) {
        STR_T result{};
        result.reserve(s.size());
        int increment{};
        wchar_t c{};
        std::int64_t len{(std::int64_t)s.size()};
        for(
            char const *utf8{s.data()};
            len > 0 && ((c = utf8_to_ucs_maxlen(utf8, len, &increment)) >= 0);
            utf8 += increment, len -= increment
        ) {
            result += static_cast<typename STR_T::value_type>(c);
        }
        return result;
    }

    template<typename STR_T = std::wstring>
    STR_T from_utf8(char const *s) {
        STR_T result{};
        int increment{};
        int c{};
        std::int64_t len{(std::int64_t)std::strlen(s)};
        result.reserve(len);
        for(
            char const *utf8{s};
            len > 0 && ((c = utf8_to_ucs_maxlen(utf8, len, &increment)) > 0);
            utf8 += increment, len -= increment
        ) {
            result += static_cast<typename STR_T::value_type>(c);
        }
        return result;
    }


    template<typename STR_T = std::wstring>
    STR_T from_utf8(char const *s, std::size_t len) {
        STR_T result{};
        int increment{};
        int c{};
        result.reserve(len);
        for(
            char const *utf8{s};
            len > 0 && ((c = utf8_to_ucs_maxlen(utf8, len, &increment)) >= 0);
            utf8 += increment, len -= increment
        ) {
            result += static_cast<typename STR_T::value_type>(c);
        }
        return result;
    }


    template<typename STR_T = std::wstring>
    void from_utf8(const std::string &s, STR_T &result) {
        result.clear();
        int increment{};
        int c{};
        std::int64_t len{(std::int64_t)s.size()};
        result.reserve(len);
        for(
            char const *utf8{s.data()};
            len > 0 && ((c = utf8_to_ucs_maxlen(utf8, len, &increment)) >= 0);
            utf8 += increment, len -= increment
        ) {
            result += static_cast<typename STR_T::value_type>(c);
        }
    }

    template<typename STR_T = std::wstring>
    void from_utf8(const std::string_view &s, STR_T &result) {
        result.clear();
        int increment{};
        int c{};
        std::int64_t len{(std::int64_t)s.size()};
        result.reserve(len);
        for(
            char const *utf8{s.data()};
            len > 0 && ((c = utf8_to_ucs_maxlen(utf8, len, &increment)) >= 0);
            utf8 += increment, len -= increment
        ) {
            result += static_cast<typename STR_T::value_type>(c);
        }
    }

    template<typename STR_T = std::wstring>
    void from_utf8(const char *s, STR_T &result) {
        result.clear();
        int increment{};
        int c{};
        std::int64_t len{(std::int64_t)std::strlen(s)};
        result.reserve(len);
        for(
            char const *utf8{s};
            len > 0 && ((c = utf8_to_ucs_maxlen(utf8, len, &increment)) > 0);
            utf8 += increment, len -= increment
        ) {
            result += static_cast<typename STR_T::value_type>(c);
        }
    }

    template<typename string_class>
    std::vector<string_class> str_tok(string_class const &str, string_class const &separator, bool separator_entire_str = true) {
        std::vector<string_class> result{};
        typename string_class::size_type begin_pos = 0;
        typename string_class::size_type end_pos = 0;
        while(begin_pos <= str.size()) {
            end_pos = separator_entire_str ? str.find(separator, begin_pos) : str.find_first_of(separator, begin_pos);
            if(end_pos != string_class::npos) {
                result.push_back(str.substr(begin_pos, end_pos - begin_pos));
            } else {
                result.push_back(str.substr(begin_pos));
                break;
            }
            begin_pos = separator_entire_str ? (end_pos + separator.size()) : (end_pos + 1);
        }

        return result;
    }

    template<typename string_class>
    std::vector<string_class> str_tok(const string_class &str, std::function<bool(std::uint64_t)>comp_func) {
        std::vector<string_class> result;
        typename string_class::size_type begin_pos = 0;
        typename string_class::size_type end_pos = 0;
        while(end_pos < str.size()) {
            if(comp_func(str[end_pos])) {
                result.push_back(str.substr(begin_pos, end_pos - begin_pos));
                end_pos = begin_pos = end_pos + 1;
            } else {
                ++end_pos;
            }
        }
        result.push_back(str.substr(begin_pos, end_pos - begin_pos));
        return result;
    }

    static std::string hexdump(const void *d, std::size_t s, std::size_t cols = 16, const std::string &splitter = "  ", bool control_dots = true) {
        std::stringstream res;
        if(s && d && cols) {
            if(!cols) {
                cols = 16;
            }
            const std::uint8_t *bd = (const std::uint8_t *)d;

            std::size_t k = s / cols;
            k *= cols;
            std::string max_offs_string;
            if(k == 0) {
                max_offs_string = "000";
            } else {
                max_offs_string = scfx::str_util::utoa<std::string>((k - 1), 16);
            }
            std::size_t moss = max_offs_string.size() + 1;

            for(std::size_t offs = 0; offs < s; offs += cols) {
                if(offs) {
                    res << '\n';
                }

                std::string curr_offs_string = scfx::str_util::utoa<std::string>(offs, 16);
                while(curr_offs_string.size() < moss) {
                    curr_offs_string = std::string("0") + curr_offs_string;
                }
                res << curr_offs_string << 'h';

                res << splitter;

                for(std::size_t i = 0; i < cols; i++) {
                    if(i != 0) {
                        res << " ";
                    }
                    if(i == cols / 2) {
                        res << " ";
                    }
                    if(offs + i < s) {
                        std::string curr_hex_byte = scfx::str_util::utoa<std::string>(bd[offs + i], 16);
                        while(curr_hex_byte.size() < 2) {
                            curr_hex_byte = std::string("0") + curr_hex_byte;
                        }
                        res << curr_hex_byte;
                    } else {
                        res << "  ";
                    }
                }

                res << splitter;

                char u[10];
                u[1] = 0;
                for(std::size_t i = 0; i < cols && offs + i < s; i++) {
                    unsigned char c = (unsigned char)bd[offs + i];
                    if(control_dots) {
                        u[0] = c >= ' ' && c < 128 ? c : '.';
                    } else {
                        if(c < ' ') {
                            int last{scfx::str_util::ucs_to_utf8(c + 0x2400, u)};
                            u[last] = 0;
                        } else if(c >= 128) {
                            scfx::str_util::ucs_to_utf8(c, u);
                        } else {
                            u[0] = c;
                        }
                    }
                    res << std::string{u};
                    std::memset(u, 0, sizeof(u));
                }
            }
        }
        return res.str();
    }

    static std::string hexdump(const std::vector<std::uint8_t> &d, std::size_t cols = 16, const std::string &splitter = "  ", bool control_dots = true) {
        if(d.size()) {
            return hexdump(d.data(), d.size(), cols, splitter, control_dots);
        } else {
            return std::string{};
        }
    }

    static std::string hexdump(const std::deque<std::uint8_t> &d, std::size_t cols = 16, const std::string &splitter = "  ", bool control_dots = true) {
        if(d.size()) {
            return hexdump(std::vector<std::uint8_t>{d.begin(), d.end()}, cols, splitter, control_dots);
        } else {
            return std::string{};
        }
    }

    static std::string hexdump(const std::string &d, std::size_t cols = 16, const std::string &splitter = "  ", bool control_dots = true) {
        if(d.size()) {
            return hexdump(std::vector<std::uint8_t>{d.begin(), d.end()}, cols, splitter, control_dots);
        } else {
            return std::string{};
        }
    }

    template<std::size_t ARRAY_SIZE>
    std::string hexdump(const std::array<std::uint8_t, ARRAY_SIZE> &d, std::size_t cols = 16, const std::string &splitter = "  ", bool control_dots = true) {
        return hexdump(d.data(), d.size(), cols, splitter, control_dots);
    }

    template<typename T>
    struct fltr {
        static T cast(const T& v) { return v; }
        static bool isspace(char) { return false; }
        static bool isalpha(char) { return false; }
        static bool isdigit(char) { return false; }
        static bool isalnum(char) { return false; }
        static bool ispunct(char) { return false; }
        static bool iscntrl(char) { return false; }
        static bool ishexdigit(int) { return false; }
        static bool isoctdigit(int) { return false; }
        static bool isbindigit(int) { return false; }
        static typename T::value_type tolower(typename T::value_type v) { return v; }
        static typename T::value_type toupper(typename T::value_type v) { return v; }
        static T strtolower(T v) { return v; }
        static T strtoupper(T v) { return v; }
    };

    template<>
    struct fltr<std::wstring> {
        static std::wstring cast(const std::string& v) { return scfx::str_util::from_utf8(v); }
        static std::wstring cast(const std::wstring& v) { return v; }
        static std::string utf8(std::wstring const &v) { return scfx::str_util::to_utf8(v); }
        static std::wstring wide(std::wstring const &v) { return v; }
        static bool isspace(std::wstring::value_type sym) { return scfx::str_util::iswspace(sym) != 0; }
        static bool isalpha(std::wstring::value_type sym) { return scfx::str_util::iswalpha(sym) != 0; }
        static bool isdigit(std::wstring::value_type sym) { return scfx::str_util::iswdigit(sym) != 0; }
        static bool isalnum(std::wstring::value_type sym) { return scfx::str_util::iswalnum(sym) != 0; }
        static bool ispunct(std::wstring::value_type sym) { return scfx::str_util::iswpunct(sym) != 0; }
        static bool iscntrl(std::wstring::value_type sym) { return scfx::str_util::iswcntrl(sym) != 0; }
        static bool ishexdigit(int c) { return scfx::str_util::ishex(c) != 0; }
        static bool isoctdigit(int c) { return (c >= '0' && c <= '7'); }
        static bool isbindigit(int c) { return c == '0' || c == '1'; }
        static std::wstring strtolower(std::wstring v) {
            std::transform(v.begin(), v.end(), v.begin(), [](wchar_t c){ return scfx::str_util::towlower(c); });
            return v;
        }
        static std::wstring strtoupper(std::wstring v) {
            std::transform(v.begin(), v.end(), v.begin(), [](wchar_t c){ return scfx::str_util::towupper(c); });
            return v;
        }
        static wchar_t tolower(wchar_t v) { return scfx::str_util::towlower(v); }
        static wchar_t toupper(wchar_t v) { return scfx::str_util::towupper(v); }
    };

    template<>
    struct fltr<std::string> {
        static std::string cast(const std::wstring& v) { return scfx::str_util::to_utf8(v); }
        static std::string cast(const std::string& v) { return v; }
        static std::string utf8(std::string const &v) { return v; }
        static std::wstring wide(std::string const &v) { return scfx::str_util::from_utf8(v); }
        static bool isspace(std::string::value_type sym) { return scfx::str_util::isspace(sym) != 0; }
        static bool isalpha(std::string::value_type sym) { return scfx::str_util::isalpha(sym) != 0; }
        static bool isdigit(std::string::value_type sym) { return scfx::str_util::isdigit(sym) != 0; }
        static bool isalnum(std::string::value_type sym) { return scfx::str_util::isalnum(sym) != 0; }
        static bool ispunct(std::string::value_type sym) { return scfx::str_util::ispunct(sym) != 0; }
        static bool iscntrl(std::string::value_type sym) { return scfx::str_util::iscntrl(sym) != 0; }
        static bool ishexdigit(int c) { return scfx::str_util::ishex(c) != 0; }
        static bool isoctdigit(int c) { return (c >= '0' && c <= '7'); }
        static bool isbindigit(int c) { return c == '0' || c == '1'; }
        static std::string strtolower(std::string v) {
            std::transform(v.begin(), v.end(), v.begin(), [](wchar_t c){ return scfx::str_util::tolower(c); });
            return v;
        }
        static std::string strtoupper(std::string v) {
            std::transform(v.begin(), v.end(), v.begin(), [](wchar_t c){ return scfx::str_util::toupper(c); });
            return v;
        }
        static char tolower(char v) { return scfx::str_util::tolower(v); }
        static char toupper(char v) { return scfx::str_util::toupper(v); }
    };

    template<typename STR_T>
    STR_T ltrim(STR_T const &s) {
        if(!s.empty()) {
            auto it{std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !fltr<STR_T>::isspace(ch);})};
            if(it != s.end()) {
                auto diff{it - s.begin()};
                return s.substr(diff);
            }
        }
        return {};
    }

    template<typename STR_T>
    STR_T rtrim(STR_T const &s) {
        if(!s.empty()) {
            auto it{std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !fltr<STR_T>::isspace(ch);})};
            if(it != s.rend()) {
                auto diff{s.rend() - it};
                return s.substr(0, diff);
            }
        }
        return {};
    }

    template<typename STR_T>
    STR_T trim(STR_T const &s) {
        return rtrim<STR_T>(ltrim<STR_T>(s));
    }

    template<typename STR_T>
    STR_T remove_char(STR_T const &s, typename STR_T::value_type c) {
        STR_T res{};
        if(!s.empty()) { res.reserve(s.size()); }
        for(auto &&cc: s) {
            if(cc != c) {
                res.push_back(cc);
            }
        }
        return res;
    }

}
