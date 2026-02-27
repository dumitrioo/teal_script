#pragma once

#include "commondefs.hpp"
#include "str_util.hpp"
#include "bit_util.hpp"

namespace scfx {

    namespace detail {

        static std::array<std::uint8_t, 85> constexpr base85_bintodigit{
            '!', '#', '$', '%', '&', '(', ')', '*', '+', '-',
            '.', '/', '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', ':', '<', '=', '>', '?', '@', 'A', 'B',
            'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
            'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
            'W', 'X', 'Y', 'Z', '[', ']', '^', 'a', 'b', 'c',
            'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
            'x', 'y', 'z', '{', '}',
        };

        static std::array<std::uint8_t, 256> constexpr base85_digittobin{
            str_util::create_reverse_table<std::uint8_t, 85, std::uint8_t>(base85_bintodigit, 85)
        };

        static std::string my85_enc_4(void const *data_ptr, std::size_t num) {
            if(num == 0 || num > 4) {
                return {};
            }
            std::array<std::uint8_t, 4> byte_buf{};
            std::copy(reinterpret_cast<std::uint8_t const *>(data_ptr), reinterpret_cast<std::uint8_t const *>(data_ptr) + num, byte_buf.begin());
            std::uint32_t const be_val{scfx::bit_util::swap_on_be<std::uint32_t>{*(std::uint32_t const *)byte_buf.data()}.val};
            std::uint32_t cur_val{be_val};
            std::array<int, 5> res_nums{};
            for(size_t i{0}; i < 5; ++i) {
                res_nums[i] = cur_val % 85;
                cur_val /= 85;
            }
            std::string res{};
            for(size_t i{0}; i < num + 1; ++i) {
                res += (char)base85_bintodigit[res_nums[i]];
            }
            return res;
        }

        struct my85_enc4_res {
            std::uint8_t data[5];
            std::uint8_t num;
        };

        static my85_enc4_res my85_dec_4(char const *str_ptr) {
            if(str_ptr == nullptr) {
                return {};
            }
            std::array<int, 5> src_nums{};
            int src_nums_cnt{0};
            for(int i = 0; i < 5; ++i) {
                if(base85_digittobin[str_ptr[i]] == 85) {
                    break;
                }
                ++src_nums_cnt;
                std::uint8_t n{base85_digittobin[str_ptr[i]]};
                src_nums[i] = n < 85 ? n : 0;
            }
            uint64_t mul{1};
            uint64_t dst_num{0};
            for(int i = 0; i < src_nums_cnt; ++i) {
                dst_num += src_nums[i] * mul;
                if(dst_num > 0xffffffffULL) {
                    throw std::runtime_error{"invalid base85 string"};
                }
                mul *= 85;
            }
            my85_enc4_res res{};
            res.num = src_nums_cnt - 1;
            for(int i = 0; i < src_nums_cnt - 1; ++i) {
                res.data[i] = (dst_num >> (8 * i)) & 0xff;
            }
            return res;
        }

        static std::vector<uint8_t> my85_dec(void const *str_ptr, std::size_t num) {
            std::vector<uint8_t> res{};
            if(str_ptr == nullptr || num == 0) {
                return res;
            }
            res.reserve(num * 4 / 5 + 4);
            std::uint8_t const *chars_ptr{(std::uint8_t const *)str_ptr};
            std::size_t num_done{0};
            std::size_t num_rest{num};
            while(num_rest > 0) {
                std::size_t to_process{std::min<std::size_t>(num_rest, 5)};
                if(to_process < 5) {
                    std::array<char, 5> buf{};
                    for(std::size_t i{0}; i < to_process; ++i) {
                        buf[i] = chars_ptr[num_done + i];
                    }
                    auto v{my85_dec_4(buf.data())};
                    std::copy (v.data, v.data + v.num, std::back_inserter(res));
                } else {
                    auto v{my85_dec_4((char *)chars_ptr + num_done)};
                    std::copy (v.data, v.data + v.num, std::back_inserter(res));
                }
                num_done += to_process;
                num_rest -= to_process;
            }
            return res;
        }

        static bool my85_validate_4(char const *str_ptr) {
            if(str_ptr == nullptr) {
                return false;
            }

            std::array<int, 5> src_nums{};
            int src_nums_cnt{0};
            for(int i = 0; i < 5; ++i) {
                if(base85_digittobin[str_ptr[i]] == 85) {
                    break;
                }
                ++src_nums_cnt;
                std::uint8_t n{base85_digittobin[str_ptr[i]]};
                src_nums[i] = n < 85 ? n : 0;
            }
            uint64_t mul{1};
            uint64_t dst_num{0};
            for(int i = 0; i < src_nums_cnt; ++i) {
                dst_num += src_nums[i] * mul;
                if(dst_num > 0xffffffffULL) {
                    return false;
                }
                mul *= 85;
            }
            return true;
        }

        static bool my85_validate(void const *str_ptr, std::size_t num) {
            std::uint8_t const *chars_ptr{(std::uint8_t const *)str_ptr};
            std::size_t num_done{0};
            std::size_t num_rest{num};
            while(num_rest > 0) {
                std::size_t to_process{std::min<std::size_t>(num_rest, 5)};
                if(to_process < 5) {
                    std::array<char, 5> buf{};
                    for(std::size_t i{0}; i < to_process; ++i) {
                        buf[i] = chars_ptr[num_done + i];
                    }
                    if(!my85_validate_4(buf.data())) { return false; }
                } else {
                    if(!my85_validate_4((char *)chars_ptr + num_done)) { return false; }
                }
                num_done += to_process;
                num_rest -= to_process;
            }
            return true;
        }

    }

    static bool is_valid_base85(std::string const &may_be_b85) {
        bool result{false};
        if(may_be_b85.size()) {
            if(detail::my85_validate(may_be_b85.data(), may_be_b85.size())) {
                result = true;
            }
        }
        return result;
    }

    static std::string data_to_base85_str(void const *data_ptr, std::size_t num) {
        std::string res{};
        if(num == 0) {
            return res;
        }
        res.reserve(num * 5 / 4 + 1);
        std::uint8_t const *byte_ptr{(std::uint8_t const *)data_ptr};
        std::size_t num_done{0};
        std::size_t num_rest{num};
        while(num_rest > 0) {
            std::size_t to_process{std::min<std::size_t>(num_rest, 4)};
            res.append(detail::my85_enc_4(byte_ptr + num_done, to_process));
            num_done += to_process;
            num_rest -= to_process;
        }
        return res;
    }

    static std::string data_to_base85_str(std::vector<std::uint8_t> const &data) {
        return data_to_base85_str(data.data(), data.size());
    }

    static std::string data_to_base85_str(std::string const &in_data) {
        return data_to_base85_str(in_data.data(), in_data.size());
    }

    template<std::size_t ARRAY_SIZE>
    std::string data_to_base85_str(std::array<std::uint8_t, ARRAY_SIZE> const &data) {
        return data_to_base85_str(data.data(), data.size());
    }

    static std::vector<std::uint8_t> base85_str_to_data(std::string const &encoded_string) {
        return detail::my85_dec(encoded_string.data(), encoded_string.size());
    }

}
