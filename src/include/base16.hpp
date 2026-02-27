#pragma once

#include "commondefs.hpp"
#include "str_util.hpp"
#include "bit_util.hpp"

namespace scfx {

    std::string data_to_hex_str(const void *data, int data_size) {
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

    std::string data_to_hex_str(const std::vector<std::uint8_t> &data) {
        return data_to_hex_str(data.data(), data.size());
    }

    std::string data_to_hex_str(const std::string &data)  {
        return data_to_hex_str(data.data(), data.size());
    }

    std::vector<std::uint8_t> hex_str_to_data(const std::string &str) {
        size_t ss = str.size();
        if(ss % 2 != 0) {
            throw std::runtime_error("in hex_str_to_data(): invalid hexadecimal string size");
        }
        std::vector<std::uint8_t> res{};
        res.reserve(ss / 2);
        for(size_t i{0}; i < ss; i += 2) {
            res.push_back(str_util::atoui(str.substr(i, 2), 16));
        }
        return res;
    }

}
