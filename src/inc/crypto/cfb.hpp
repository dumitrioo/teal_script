#pragma once

#include "../commondefs.hpp"

namespace teal {

    namespace crypt {

        template<typename CIPHER>
        struct cfb {
            cfb() = default;
            cfb(CIPHER *cipher_context_ptr): cipher_{cipher_context_ptr} {}
            cfb(cfb const &) = default;
            cfb &operator=(cfb const &) = default;
            cfb(cfb &&) = default;
            cfb &operator=(cfb &&) = default;
            ~cfb() = default;

            bool encrypt(std::size_t s, void *iv, void const *p, void *c, std::size_t length) {
                size_t i;
                size_t n;
                uint8_t o[16];

                if((s % 8) != 0)
                   return false;

                s = s / 8;

                if(s < 1 || s > cipher_->block_size())
                   return false;

                while(length > 0) {
                   n = std::min(length, s);

                   cipher_->encrypt_block(iv, o);

                   for(i = 0; i < n; i++) {
                      ((uint8_t *)c)[i] = ((uint8_t const *)p)[i] ^ o[i];
                   }

                   std::memmove(iv, (uint8_t *)iv + s, cipher_->block_size() - s);
                   std::memcpy((uint8_t *)iv + cipher_->block_size() - s, c, s);

                   p = ((uint8_t const *)p) + n;
                   c = ((uint8_t *)c) + n;
                   length -= n;
                }

                return true;
            }

            bool decrypt(std::size_t s, void *iv, const void *c, void *p, std::size_t length) {
                size_t i;
                size_t n;
                uint8_t o[16];

                if((s % 8) != 0) {
                   return false;
                }

                s = s / 8;

                if(s < 1 || s > cipher_->block_size()) {
                   return false;
                }

                while(length > 0) {
                   n = std::min(length, s);

                   cipher_->encrypt_block(iv, o);

                   std::memmove(iv, (uint8_t *)iv + s, cipher_->block_size() - s);
                   std::memcpy((uint8_t *)iv + cipher_->block_size() - s, c, s);

                   for(i = 0; i < n; i++) {
                      ((uint8_t *)p)[i] = ((uint8_t const *)c)[i] ^ o[i];
                   }

                   c = ((uint8_t const *)c) + n;
                   p = ((uint8_t *)p) + n;

                   length -= n;
                }

                return true;
            }

        private:
            CIPHER *cipher_{nullptr};
        };

    }

}
