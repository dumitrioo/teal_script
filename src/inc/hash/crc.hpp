#pragma once

#include "../commondefs.hpp"
#include "../bit_util.hpp"

namespace scfx {

    template <typename T, T POLYNOMIAL, T INITIAL_REMAINDER, bool REFLECT_DATA, bool REFLECT_REMAINDER, T FINAL_XOR_VALUE>
    class crc_calc {
        static constexpr T WIDTH{8 * sizeof(T)};
        static constexpr T TOPBIT{(T)1 << (WIDTH - 1)};

    public:
        T calculate(void const *message, size_t num_bytes) const {
            uint8_t const *bytes{(uint8_t const *)message};
            T remainder{INITIAL_REMAINDER};
            if constexpr (REFLECT_DATA) {
                for(size_t byte{0}; byte < num_bytes; ++byte) {
                    std::uint8_t data{(std::uint8_t)(bit_util::byte_flip(bytes[byte]) ^ (remainder >> (WIDTH - 8)))};
                    remainder = crc_table_[data] ^ (remainder << 8);
                }
            } else {
                for(size_t byte{0}; byte < num_bytes; ++byte) {
                    std::uint8_t data = (bytes[byte]) ^ (remainder >> (WIDTH - 8));
                    remainder = crc_table_[data] ^ (remainder << 8);
                }
            }
            if constexpr (REFLECT_REMAINDER) {
                reflect(&remainder, WIDTH / 8);
            }
            return remainder ^ FINAL_XOR_VALUE;
        }

    private:
        static void reflect(void *data, std::size_t len) {
            std::uint8_t *bytes{(std::uint8_t *)data};
            bit_util::inplace_swap(bytes, len);
            for(std::size_t i{0}; i < len; ++i) {
                bytes[i] = bit_util::byte_flip(bytes[i]);
            }
        }

        static std::array<T, 256> constexpr fresh_table() {
            std::array<T, 256> res{};
            for(size_t dividend{0}; dividend < 256; ++dividend) {
                T remainder{static_cast<T>(dividend) << (WIDTH - 8)};
                for(size_t bit = 8; bit > 0; --bit) {
                    if(remainder & TOPBIT) {
                        remainder = (remainder << 1) ^ POLYNOMIAL;
                    } else {
                        remainder = (remainder << 1);
                    }
                }
                res[dividend] = remainder;
            }
            return res;
        }

        static std::array<T, 256> constexpr crc_table_{fresh_table()};
    };

    using crc8 	        = crc_calc<uint8_t, 0x7, 0x0, false, false, 0x0>;
    using crc8_cdma2000 = crc_calc<uint8_t, 0x9b, 0xff, false, false, 0x0>;
    using crc8_darc     = crc_calc<uint8_t, 0x39, 0x0, true, true, 0x0>;
    using crc8_dvb_s2   = crc_calc<uint8_t, 0xd5, 0x0, false, false, 0x0>;
    using crc8_ebu      = crc_calc<uint8_t, 0x1d, 0xff, true, true, 0x0>;
    using crc8_i_code   = crc_calc<uint8_t, 0x1d, 0xfd, false, false, 0x0>;
    using crc8_itu      = crc_calc<uint8_t, 0x7, 0x0, false, false, 0x55>;
    using crc8_maxim    = crc_calc<uint8_t, 0x31, 0x0, true, true, 0x0>;
    using crc8_rohc     = crc_calc<uint8_t, 0x7, 0xff, true, true, 0x0>;
    using crc8_wcdma    = crc_calc<uint8_t, 0x9b, 0x0, true, true, 0x0>;

    using crc16_arc = crc_calc<uint16_t, 0x8005, 0x0000, true, true, 0x0000>;
    using crc16_aug_ccitt = crc_calc<uint16_t, 0x1021, 0x1d0f, false, false, 0x0000>;
    using crc16_buypass = crc_calc<uint16_t, 0x8005, 0x0000, false, false, 0x0000>;
    using crc16_ccitt_false = crc_calc<uint16_t, 0x1021, 0xffff, false, false, 0x0000>;
    using crc16_cdma2000 = crc_calc<uint16_t, 0xc867, 0xffff, false, false, 0x0000>;
    using crc16_dds_110 = crc_calc<uint16_t, 0x8005, 0x800d, false, false, 0x0000>;
    using crc16_dect_r = crc_calc<uint16_t, 0x0589, 0x0000, false, false, 0x1>;
    using crc16_dect_x = crc_calc<uint16_t, 0x0589, 0x0000, false, false, 0x0000>;
    using crc16_dnp = crc_calc<uint16_t, 0x3d65, 0x0, true, true, 0xffff>;
    using crc16_en_13757 = crc_calc<uint16_t, 0x3d65, 0x0000, false, false, 0xffff>;
    using crc16_genibus = crc_calc<uint16_t, 0x1021, 0xffff, false, false, 0xffff>;
    using crc16_maxim = crc_calc<uint16_t, 0x8005, 0x0000, true, true, 0xffff>;
    using crc16_mcrf4xx = crc_calc<uint16_t, 0x1021, 0xffff, true, true, 0x0000>;
    using crc16_riello = crc_calc<uint16_t, 0x1021, 0xb2aa, true, true, 0x0000>;
    using crc16_t10_dif = crc_calc<uint16_t, 0x8bb7, 0x0000, false, false, 0x0000>;
    using crc16_teledisk = crc_calc<uint16_t, 0xa097, 0x0000, false, false, 0x0000>;
    using crc16_tms37157 = crc_calc<uint16_t, 0x1021, 0x89ec, true, true, 0x0000>;
    using crc16_usb = crc_calc<uint16_t, 0x8005, 0xffff, true, true, 0xffff>;
    using crc16_a = crc_calc<uint16_t, 0x1021, 0xc6c6, true, true, 0x0000>;
    using crc16_kermit = crc_calc<uint16_t, 0x1021, 0x0000, true, true, 0x0000>;
    using crc16_modbus = crc_calc<uint16_t, 0x8005, 0xffff, true, true, 0x0000>;
    using crc16_x_25 = crc_calc<uint16_t, 0x1021, 0xffff, true, true, 0xffff>;
    using crc16_xmodem = crc_calc<uint16_t, 0x1021, 0x0000, false, false, 0x0000>;

    using crc32 = crc_calc<uint32_t, 0x04c11db7u, 0xffffffffu, true, true, 0xffffffffu>;
    using crc32_zlib = crc_calc<uint32_t, 0x04c11db7u, 0xffffffffu, true, true, 0xffffffffu>;
    using crc32_bzip2 = crc_calc<uint32_t, 0x04c11db7u, 0xffffffffu, false, false, 0xffffffffu>;
    using crc32_c = crc_calc<uint32_t, 0x1edc6f41u, 0xffffffffu, true, true, 0xffffffffu>;
    using crc32_d = crc_calc<uint32_t, 0xa833982bu, 0xffffffffu, true, true, 0xffffffffu>;
    using crc32_mpeg2 = crc_calc<uint32_t, 0x04c11db7u, 0xffffffffu, false, false, 0x00000000u>;
    using crc32_posix = crc_calc<uint32_t, 0x04c11db7u, 0x00000000u, false, false, 0xffffffffu>;
    using crc32_q = crc_calc<uint32_t, 0x814141abu, 0x00000000u, false, false, 0x00000000u>;
    using crc32_jamcrc = crc_calc<uint32_t, 0x04c11db7u, 0xffffffffu, true, true, 0x00000000u>;
    using crc32_xfer = crc_calc<uint32_t, 0xafu, 0x00000000u, false, false, 0x00000000u>;

    using crc64 = crc_calc<uint64_t, 0x42f0e1eba9ea3693ull, 0x0ull, false, false, 0x0ull>;
    using crc64_we = crc_calc<uint64_t, 0x42f0e1eba9ea3693ull, 0xffffffffffffffffull, false, false, 0xffffffffffffffffull>;
    using crc64_xz = crc_calc<uint64_t, 0x42f0e1eba9ea3693ull, 0xffffffffffffffffull, true, true, 0xffffffffffffffffull>;

}
