#pragma once

#include "commondefs.hpp"
#include "sys_util.hpp"

namespace teal::bit_util {

    static std::uint8_t byte_flip(std::uint8_t n) {
        constexpr std::uint64_t ta{17848844570815808640ULL};
        return (((ta >> ((n & 0xf) << 2)) & 0xf) << 4) | ((ta >> (((n >> 4) & 0xf) << 2)) & 0xf);
    }


    template<typename T>
    [[nodiscard]]
    constexpr T byteswap(T val) noexcept {
#if (__cplusplus < 202300L)
        if constexpr (sizeof(T) == 1) {
            return val;
        }
        std::uint8_t const *val_ptr{reinterpret_cast<std::uint8_t const *>(&val)};
        std::uint8_t ret_val_buf[sizeof(T)];
        for(size_t i{}; i < sizeof(T); ++i) {
            ret_val_buf[sizeof(T) - i - 1] = val_ptr[i];
        }
        return *reinterpret_cast<T *>(&ret_val_buf[0]);
#else
        return std::byteswap(val);
#endif
    }


    static constexpr void inplace_swap(void *data, std::size_t count) {
        if(data && count > 1) {
            std::size_t const count_half{count >> 1};
            for(std::size_t i{0}; i < count_half; ++i) {
                std::swap(
                    *(reinterpret_cast<std::uint8_t *>(data) + i),
                    *(reinterpret_cast<std::uint8_t *>(data) + count - 1 - i)
                );
            }
        }
    }


    template<typename T> struct rol;
    template<> struct rol<std::uint8_t> { std::uint8_t val; rol(std::uint8_t v, std::size_t n): val{(std::uint8_t)((v << n) | (v >> (8 - n)))} {} };
    template<> struct rol<std::int8_t> { std::int8_t val; rol(std::int8_t v, std::size_t n): val{(std::int8_t)(((std::uint8_t)v << n) | (((std::uint8_t)v) >> (8 - n)))} {} };
    template<> struct rol<std::uint16_t> { std::uint16_t val; rol(std::uint16_t v, std::size_t n): val{(std::uint16_t)(((std::uint16_t)v << n) | ((std::uint16_t)v >> (16 - n)))} {} };
    template<> struct rol<std::int16_t> { std::int16_t val; rol(std::int16_t v, std::size_t n): val{(std::int16_t)(((std::uint16_t)v << n) | (((std::uint16_t)v) >> (16 - n)))} {} };
    template<> struct rol<std::uint32_t> { std::uint32_t val; rol(std::uint32_t v, std::size_t n): val{(v << n) | (v >> (32 - n))} {} };
    template<> struct rol<std::int32_t> { std::int32_t val; rol(std::int32_t v, std::size_t n): val{(std::int32_t)(((std::uint32_t)v << n) | (((std::uint32_t)v) >> (32 - n)))} {} };
    template<> struct rol<std::uint64_t> { std::uint64_t val; rol(std::uint64_t v, std::size_t n): val{(v << n) | (v >> (64 - n))} {} };
    template<> struct rol<std::int64_t> { std::int64_t val; rol(std::int64_t v, std::size_t n): val{(std::int64_t)(((std::uint64_t)v << n) | (((std::uint64_t)v) >> (64 - n)))} {} };

    template<typename T> struct ror;
    template<> struct ror<std::uint8_t> { std::uint8_t val; ror(std::uint8_t v, std::size_t n): val{(std::uint8_t)((v >> n) | (v << (8 - n)))} {} };
    template<> struct ror<std::int8_t> { std::int8_t val; ror(std::int8_t v, std::size_t n): val{(std::int8_t)((((std::uint8_t)v) >> n) | (((std::uint8_t)v) << (8 - n)))} {} };
    template<> struct ror<std::uint16_t> { std::uint16_t val; ror(std::uint16_t v, std::size_t n): val{(std::uint16_t)((v >> n) | (v << (16 - n)))} {} };
    template<> struct ror<std::int16_t> { std::int16_t val; ror(std::int16_t v, std::size_t n): val{(std::int16_t)((((std::uint16_t)v) >> n) | (((std::uint16_t)v) << (16 - n)))} {} };
    template<> struct ror<std::uint32_t> { std::uint32_t val; ror(std::uint32_t v, std::size_t n): val{((v) >> n) | ((v) << (32 - n))} {} };
    template<> struct ror<std::int32_t> { std::int32_t val; ror(std::int32_t v, std::size_t n): val{(std::int32_t)((((std::uint32_t)v) >> n) | (((std::uint32_t)v) << (32 - n)))} {} };
    template<> struct ror<std::uint64_t> { std::uint64_t val; ror(std::uint64_t v, std::size_t n): val{(v >> n) | (v << (64 - n))} {} };
    template<> struct ror<std::int64_t> { std::int64_t val; ror(std::int64_t v, std::size_t n): val{(std::int64_t)((((std::uint64_t)v) >> n) | (((std::uint64_t)v) << (64 - n)))} {} };


    template<typename T>
    struct swap_on_le {
        T val;
        swap_on_le(T v): val{v} {
            if constexpr (teal::sys_util::little_endian()) {
                val = byteswap<T>(val);
            }
        }
    };

    template<typename T>
    using hnswap = swap_on_le<T>;


    template<typename T>
    struct swap_on_be {
        T val;
        swap_on_be(T v): val{v} {
            if constexpr (teal::sys_util::big_endian()) {
                val = byteswap<T>(val);
            }
        }
    };


    template<class T>
    T from_bytes(void const *v) { return *reinterpret_cast<T const *>(v); }
    template<class T>
    void *to_bytes(T const &v, void *m_ptr) {
        *reinterpret_cast<T *>(m_ptr) = swap_on_le{v}.val;
        return reinterpret_cast<std::uint8_t *>(m_ptr) + sizeof(T);
    }

    template<class T>
    T from_net_bytes(void const *v) { return swap_on_le{from_bytes<T>(v)}.val; }
    template<class T>
    void *to_net_bytes(T const &v, void *m_ptr) {
        *reinterpret_cast<T *>(m_ptr) = swap_on_le{v}.val;
        return reinterpret_cast<std::uint8_t *>(m_ptr) + sizeof(T);
    }


    template<typename T>
    class bits {
    public:
        bits() = default;
        bits(bits const &that) = default;
        bits(bits &&that) = default;
        bits &operator=(bits const &that) = default;
        bits &operator=(bits &&that) = default;
        ~bits() = default;

        bits(T val): val_{val} {}

        template<typename U>
        bits(U val): val_{static_cast<T>(val)} {}

        template<typename U>
        bits &operator=(U val) {
            val_ = static_cast<T>(val);
            return *this;
        }

        void set(size_t at) {
            if(at < sizeof(T) * 8) {
                val_ |= static_cast<T>(1) << at;
            }
        }

        void clr(size_t at) {
            if(at < sizeof(T) * 8) {
                val_ &= ~(static_cast<T>(1) << at);
            }
        }

        bool is_set(size_t at) const {
            return (at < sizeof(T) * 8) ? (((val_ >> at) & 1) == 1) : false;
        }

        bool is_clr(size_t at) const {
            return !is_set(at);
        }

        T get(size_t at) const {
            return is_set(at) ? 1 : 0;
        }

        void set(size_t bit_width, size_t lshift, T val) {
            if(
                bit_width > 0 &&
                lshift < sizeof(T) * 8 &&
                lshift + bit_width <= sizeof(T) * 8
            ) {
                T res{val_};
                res &= ~(lo_mask(bit_width) << lshift);
                res |= (val & lo_mask(bit_width)) << lshift;
                res = val_;
            }
        }

        T get(size_t lshift, size_t bit_width) const {
            T res{val_};
            if(lshift >= sizeof(T) * 8 || bit_width == 0) {
                return 0;
            }
            if(lshift + bit_width > sizeof(T) * 8) {
                bit_width = sizeof(T) * 8 - lshift;
            }
            if(lshift == 0 && bit_width == sizeof(T) * 8) {
                return val_;
            }
            if(lshift + bit_width < sizeof(T) * 8 + 1) {
                res >>= lshift;
                T msk{static_cast<T>(lo_mask(bit_width))};
                res &= msk;
            }
            return res;
        }

        T whole() const {
            return val_;
        }

    private:
        static inline std::uint8_t const onesrc[16] {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        };

        static T lo_mask(size_t bit_width) {
            if(bit_width == sizeof(T) * 8) {
                return *reinterpret_cast<T const *>(onesrc);
            }
            return static_cast<T>(static_cast<T>(1) << bit_width) - 1;
        }

    private:
        T val_{};
    };

}
