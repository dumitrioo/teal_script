#pragma once

#include "../commondefs.hpp"
#include "adler.hpp"
#include "crc.hpp"

namespace teal {

    template<typename T>
#if (__cplusplus >= 202000L)
        requires(std::is_integral_v<T> || std::is_enum_v<T>)
#endif
    struct num_cast_hash {
        std::size_t operator()(T n) const noexcept {
            return static_cast<std::size_t>(n);
        }
    };

    template<typename STR_T>
#if (__cplusplus >= 202000L)
        requires(std::is_fundamental_v<typename STR_T::value_type>)
#endif
    struct str_hash {
        std::size_t operator()(STR_T const &s) const noexcept {
            using char_type = uintn_t<sizeof(typename STR_T::value_type)>;
            std::size_t res{0};
            std::size_t const ss{s.size()};
            char_type const *sd{reinterpret_cast<char_type const *>(s.data())};
            for(std::size_t i{0}; i < ss; i += sizeof(std::size_t) / sizeof(char_type)) {
                std::size_t k{0};
                std::size_t const end{std::min<std::size_t>(sizeof(std::size_t) / sizeof(char_type), ss - i)};
                for(std::size_t j{0}; j < end; ++j) {
                    std::size_t const indx{i + j};
                    std::size_t l{static_cast<std::size_t>(sd[indx])};
                    l <<= j * (sizeof(char_type) * 8);
                    k |= l;
                }
                res += k;
            }
            return res;
        }
    };

    template<typename STR_T>
#if (__cplusplus >= 202000L)
        requires(std::is_fundamental_v<typename STR_T::value_type>)
#endif
    struct str_crc {
        std::size_t operator()(STR_T const &s) const noexcept {
            return crc_.calculate(s.data(), s.size() * sizeof(typename STR_T::value_type));
        }
        crc64 crc_{};
    };

}
