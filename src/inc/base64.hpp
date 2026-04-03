#pragma once

#include "commondefs.hpp"
#include "str_util.hpp"

namespace teal {

    namespace detail {

        static constexpr std::array<std::uint8_t, 64>base64_chars{
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
        };

        static constexpr std::array<int, 256> base64_remap{
            str_util::create_reverse_table<std::uint8_t, 64, int>(base64_chars, -1)
        };

        static std::string data_to_base64_str(void const *in_data, std::size_t in_len) {
            std::string ret{};
            ret.reserve(in_len * 4 / 3 + 16);
            std::size_t i{0};
            std::size_t j{0};
            std::uint8_t char_array_3[3]{};
            std::uint8_t char_array_4[4]{};
            std::uint8_t const *bytes_to_encode{(std::uint8_t const *)in_data};

            while(in_len--) {
                char_array_3[i++] = *(bytes_to_encode++);
                if(i == 3) {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;

                    for(i = 0; i < 4; i++) {
                        ret += base64_chars[char_array_4[i]];
                    }
                    i = 0;
                }
            }

            if(i) {
                for(j = i; j < 3; j++) {
                    char_array_3[j] = 0;
                }

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(j = 0; j < i + 1; j++) {
                    ret += base64_chars[char_array_4[j]];
                }

                while(i++ < 3) {
                    ret += '=';
                }

            }
            return ret;
        }

        static std::string data_to_base64_str(std::vector<std::uint8_t> const &data) {
            return data_to_base64_str(data.data(), data.size());
        }

        static bool is_base64(unsigned char c) {
            return (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 47 && c <= 57) || c == 43;
        }

        static std::vector<std::uint8_t> base64_str_to_data(std::string const &encoded_string) {
            if(encoded_string.empty()) {
                return {};
            }

            if(encoded_string.size() % 4 != 0) {
                throw std::runtime_error{"wrong base64 string size"};
            }

            std::size_t in_len{encoded_string.size()};
            std::size_t i{0};
            std::size_t j{0};
            std::size_t in_{0};
            std::uint8_t char_array_4[4]{};
            std::uint8_t char_array_3[3]{};
            std::vector<std::uint8_t> ret{};
            ret.reserve(encoded_string.size() * 3 / 4 + 16);

            while(in_len-- && (encoded_string[in_] != '=')) {
                if(!is_base64(encoded_string[in_])) {
                    if(std::iscntrl(encoded_string[in_]) || std::isspace(encoded_string[in_])) {
                        ++in_;
                        continue;
                    } else {
                        throw std::runtime_error{"invalid base64 string"};
                    }
                }
                char_array_4[i++] = encoded_string[in_]; in_++;
                if(i == 4) {
                    for(i = 0; i < 4; i++) {
                        char_array_4[i] = base64_remap[char_array_4[i]];
                    }

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    ret.insert(ret.end(), char_array_3, char_array_3 + 3);

                    i = 0;
                }
            }

            if(i) {
                for(j = i; j < 4; j++) {
                    char_array_4[j] = 0;
                }

                for(j = 0; j < 4; j++) {
                    char_array_4[j] = base64_remap[char_array_4[j]];
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for(j = 0; j < i - 1; j++) {
                    ret.push_back(char_array_3[j]);
                }
            }

            return ret;
        }

        static bool is_valid_base64(std::string const &may_be_b64) {
            bool result{false};
            try {
                if(may_be_b64.size()) {
                    auto recovered_data{base64_str_to_data(may_be_b64)};
                    if(recovered_data.size()) {
                        result = true;
                    }
                }
            } catch (...) {
                result = false;
            }
            return result;
        }

    }

    static std::string data_to_base64_str(void const *in_data, std::size_t in_len) {
#ifdef USE_VEX_BASE64
        if(in_data && in_len) {
            std::string res{};
            size_t new_len{in_len * 4 / 3};
            size_t new_len_r{new_len % 4 ? 4 - new_len % 4 : 0};
            res.resize(new_len + new_len_r);
            size_t outlen;
            base64_encode((char const *)in_data, in_len, res.data(), &outlen, 0);
            return res;
        }
        return {};
#else
        return detail::data_to_base64_str(in_data, in_len);
#endif

    }

    static std::string data_to_base64_str(std::vector<std::uint8_t> const &in_data) {
        return data_to_base64_str(in_data.data(), in_data.size());
    }

    static std::vector<std::uint8_t> base64_str_to_data(std::string const &encoded_string) {
#ifdef USE_VEX_BASE64
        std::vector<std::uint8_t> res{};
        if(!encoded_string.empty()) {
            res.resize(encoded_string.size() * 3 / 4 + 4);
            size_t outlen;
            int decres{base64_decode((char const *)encoded_string.data(), encoded_string.size(), (char *)res.data(), &outlen, 0)};
            if(decres == 1) {
                res.resize(outlen);
            } else {
                throw std::runtime_error{"invalid base64 string"};
            }
        }
        return res;
#else
        return detail::base64_str_to_data(encoded_string);
#endif
    }

}
