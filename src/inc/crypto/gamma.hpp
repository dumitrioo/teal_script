#pragma once

#include "../commondefs.hpp"
#include "../base16.hpp"
#include "threefish.hpp"

namespace teal::crypt {

    class gamma final {
    public:
        gamma() {
            std::array<std::uint8_t, teal::crypt::threefish<512>::key_length()> key{};
            cipher_.init(key.data(), nullptr);
        }

        gamma(void const *key, std::size_t k_size, void const *iv, std::size_t iv_size) {
            init(key, k_size, iv, iv_size);
        }

        gamma(teal::bytevec const &key, teal::bytevec const &iv) {
            init(key, iv);
        }

        gamma(gamma const &) = default;
        gamma(gamma &&) = default;
        gamma &operator=(gamma const &) = default;
        gamma &operator=(gamma &&) = default;
        ~gamma() = default;

        static std::size_t constexpr key_size() {
            return teal::crypt::threefish<512>::key_length();
        }

        static std::size_t constexpr block_size() {
            return teal::crypt::threefish<512>::block_size();
        }

        static std::size_t constexpr iv_length() {
            return teal::crypt::threefish<512>::iv_length() - 8;
        }

        void init(void const *key, std::size_t k_size, void const *iv = nullptr, std::size_t iv_size = 0) {
            set_key(key, k_size);
            set_iv(iv, iv_size);
            need_curr_block_refresh_ = true;
        }

        void init(teal::bytevec const &key, teal::bytevec const &iv = {}) {
            init(key.data(), key.size(), iv.data(), iv.size());
        }

        template<std::size_t KEYSIZE>
        void init(std::array<std::uint8_t, KEYSIZE> const &key = {}) {
            init(key.data(), key.size());
        }

        void init_hexstr(std::string const &key, std::string const &iv = {}) {
            std::vector<std::uint8_t> k{hex_str_to_data(key)};
            std::vector<std::uint8_t> i{hex_str_to_data(iv)};
            init(k.data(), k.size(), i.data(), i.size());
        }

        void init(std::string const &key, std::string const &iv = {}) {
            init(key.data(), key.size(), iv.data(), iv.size());
        }

        void set_key(void const *key, std::size_t k_size) {
            if(key && k_size) {
                if(k_size == key_size()) {
                    std::memcpy(&key_[0], key, key_size());
                } else {
                    std::memset(&key_[0], 0, key_size());
                    std::memcpy(&key_[0], key, std::min<std::size_t>(k_size, key_size()));
                }
            } else {
                std::memset(&key_[0], 0, k_size);
            }
            cipher_.init(key_.data(), iv_.data());
            need_curr_block_refresh_ = true;
        }

        void set_iv(void const *iv, std::size_t iv_size) {
            if(iv && iv_size) {
                if(iv_size == iv_length()) {
                    std::memcpy(&iv_[8], iv, iv_length());
                } else {
                    std::memset(&iv_[8], 0, iv_length());
                    std::memcpy(&iv_[8], iv, std::min<std::size_t>(iv_length(), iv_size));
                }
            } else {
                std::memset(&iv_[8], 0, iv_length());
            }
            merge_iv();
            cipher_.init(key_.data(), iv_.data());
            need_curr_block_refresh_ = true;
        }

        std::uint8_t operator[](std::uint64_t indx) {
            std::uint64_t blk_no{indx / block_size()};
            if(curr_block_no_ != blk_no || need_curr_block_refresh_) {
                curr_block_no_ = blk_no;
                merge_iv();
                cipher_.init(key_.data(), iv_.data());
                std::array<std::uint8_t, block_size()> inb{};
                cipher_.encrypt_block(inb.data(), &curr_block_[0]);
                need_curr_block_refresh_ = false;
            }
            return curr_block_[indx % block_size()];
        }

        void clear() {
            curr_block_no_ = 0xffffffffffffffffULL;
            curr_block_ = {};
            std::array<std::uint8_t, teal::crypt::threefish<512>::key_length()> key{};
            cipher_.init(key.data(), nullptr);
            need_curr_block_refresh_ = true;
        }

    private:
        void merge_iv() {
            std::uint64_t curr_block_no{teal::bit_util::swap_on_be<std::uint64_t>{curr_block_no_}.val};
            memcpy(&iv_[0], &curr_block_no, 8);
        }

    private:
        std::array<std::uint8_t, teal::crypt::threefish<512>::key_length()> key_{};
        std::array<std::uint8_t, teal::crypt::threefish<512>::iv_length()> iv_{};
        std::uint64_t curr_block_no_{0xffffffffffffffffULL};
        std::array<std::uint8_t, teal::crypt::threefish<512>::block_size()> curr_block_{};
        teal::crypt::threefish<512> cipher_{};
        bool need_curr_block_refresh_{true};
    };


}
