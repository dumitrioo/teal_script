#pragma once

#include "../commondefs.hpp"

namespace teal {

    namespace crypt {

        template<typename CIPHER>
        struct cbc {
            cbc() = default;
            cbc(CIPHER *cipher_context_ptr): cipher_{cipher_context_ptr} {}
            cbc(cbc const &) = default;
            cbc &operator=(cbc const &) = default;
            cbc(cbc &&) = default;
            cbc &operator=(cbc &&) = default;
            ~cbc() = default;

            bool encrypt(void *iv, void const *p, void *c, std::size_t length) {
                size_t i;
                while(length >= cipher_->block_size()) {
                   for(i = 0; i < cipher_->block_size(); i++) {
                      ((uint8_t *)c)[i] = ((uint8_t const *)p)[i] ^ ((uint8_t *)iv)[i];
                   }
                   cipher_->encrypt_block((std::uint8_t const *)c, (std::uint8_t *)c);
                   std::memcpy(iv, c, cipher_->block_size());
                   p = ((char const *)p) + cipher_->block_size();
                   c = ((char *)c) + cipher_->block_size();
                   length -= cipher_->block_size();
                }
                if(length != 0) {
                    return false;
                }
                return true;
            }

            bool decrypt(void *iv, const void *c, void *p, std::size_t length) {
                size_t i;
                uint8_t t[16];
                while(length >= cipher_->block_size()) {
                    std::memcpy(t, c, cipher_->block_size());
                    cipher_->decrypt_block((std::uint8_t const *)c, (std::uint8_t *)p);
                    for(i = 0; i < cipher_->block_size(); i++) {
                        ((uint8_t *)p)[i] ^= ((uint8_t *)iv)[i];
                    }
                    std::memcpy(iv, t, cipher_->block_size());
                    c = ((char const *)c) + cipher_->block_size();
                    p = ((char *)p) + cipher_->block_size();
                    length -= cipher_->block_size();
                }
                if(length != 0) {
                   return false;
                }
                return true;
            }

        private:
            CIPHER *cipher_{nullptr};
        };

    }

}
