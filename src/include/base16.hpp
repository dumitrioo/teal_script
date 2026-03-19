#pragma once

#include "commondefs.hpp"
#include "str_util.hpp"
#include "bit_util.hpp"

namespace scfx {

    static std::string data_to_hex_str(const void *data, int data_size) {
        static char hex_digits[] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        if(data == 0 || data_size == 0) {
            return std::string{};
        }
        std::string result{};
        result.reserve(data_size * 2);
        const std::uint8_t *byte_data = reinterpret_cast<const std::uint8_t *>(data);
        for(int i{0}; i < data_size; i++) {
            std::uint8_t byte = byte_data[i];
            result.push_back(hex_digits[(byte >> 4) & 0x0f]);
            result.push_back(hex_digits[byte & 0x0f]);
        }
        return result;
    }

    static std::string data_to_hex_str(const std::vector<std::uint8_t> &data) {
        return data_to_hex_str(data.data(), data.size());
    }

    static std::string data_to_hex_str(const std::string &data)  {
        return data_to_hex_str(data.data(), data.size());
    }

    template<std::size_t ARRAY_SIZE>
    std::string data_to_hex_str(const std::array<std::uint8_t, ARRAY_SIZE> &data) {
        return data_to_hex_str(data.data(), data.size());
    }

    static std::vector<std::uint8_t> hex_str_to_data(const std::string &str) {
        static constexpr std::int16_t hex_do_digit[256] {
  /*       0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
  /*00*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-1,-1,
  /*10*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,
  /*20*/  -2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*30*/   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
  /*40*/  -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*50*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*60*/  -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*70*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*80*/  -1,-1,-1,-1,-1,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*90*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*a0*/  -2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*b0*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*c0*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*d0*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*e0*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  /*f0*/  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
        };
        size_t ss = str.size();
        std::vector<std::uint8_t> res{};
        res.reserve(ss / 2 + 2);
        std::string buf{};
        for(size_t i{0}; i < ss; ++i) {
            auto asnum{hex_do_digit[static_cast<std::uint8_t>(str[i])]};
            if(asnum >= 0) {
                buf.push_back(str[i]);
                if(buf.size() == 2) {
                    std::uint8_t c{
                        static_cast<std::uint8_t>(
                            (hex_do_digit[static_cast<std::uint8_t>(buf[0])] << 4) |
                            hex_do_digit[static_cast<std::uint8_t>(buf[1])]
                        )
                    };
                    res.push_back(c);
                    buf.clear();
                }
            } else if(asnum == -1) {
                throw std::runtime_error("invalid hexadecimal string: containing wrong characters");
            }
        }
        if(!buf.empty()) {
            throw std::runtime_error("invalid hexadecimal string: odd number of hex characters");
        }
        return res;
    }

}
