#pragma once

#include "../commondefs.hpp"

namespace teal {

    namespace crypt {

        template<typename CIPHER>
        struct ctr {
            ctr() = default;
            ctr(CIPHER *cipher_context_ptr): cipher_{cipher_context_ptr} {}
            ctr(ctr const &) = default;
            ctr &operator=(ctr const &) = default;
            ctr(ctr &&) = default;
            ctr &operator=(ctr &&) = default;
            ~ctr() = default;


            bool encrypt(std::uint32_t m, void *t, void const *p, void *c, std::size_t length) {
                if(m > cipher_->block_size()) {
                    return false;
                }

                std::uint8_t tv[16];
                std::memcpy(tv, t, 16);

                std::size_t const n{std::min<std::size_t>(length, cipher_->block_size())};
                std::array<std::uint8_t, CIPHER::block_size()> o{};

                while(length > 0) {
                    cipher_->encrypt_block(t, o.data());
                    std::memcpy(tv, t, 16);

                    for(std::size_t i{0}; i < n; i++) {
                        ((uint8_t *)c)[i] = ((uint8_t const *)p)[i] ^ o[i];
                    }

                    for(std::size_t temp = 1, i = 1; i <= m; i++) {
                        temp += ((uint8_t *)t)[cipher_->block_size() - i];
                        ((uint8_t *)t)[cipher_->block_size() - i] = temp & 0xFF;
                        std::memcpy(tv, t, 16);
                        temp >>= 8;
                    }

                    p = ((uint8_t const *)p) + n;
                    c = ((uint8_t *)c) + n;
                    length -= n;
                }

                return true;
            }

            bool decrypt(std::uint32_t m, void *t, void const *c, void *p, std::size_t length) {
                return encrypt(m, t, c, p, length);
            }

        private:
            CIPHER *cipher_{nullptr};
        };

    }

}
