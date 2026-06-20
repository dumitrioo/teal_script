#pragma once

#include "../commondefs.hpp"
#include "sha3_512.hpp"
#include "whirlpool.hpp"
#include "../bit_util.hpp"

namespace teal {

    namespace crypt {

        class kecipher {
        public:
            kecipher() = default;
            kecipher(void const *keyData, void const *tweakData) { init(keyData, tweakData); }
            kecipher(kecipher const &) = default;
            kecipher &operator=(kecipher const &) = default;
            kecipher(kecipher &&) = default;
            kecipher &operator=(kecipher &&) = default;
            ~kecipher() = default;

            static constexpr std::uint32_t block_size() {
                return 64;
            }

            static constexpr std::uint32_t key_length() {
                return 64;
            }

            static constexpr std::uint32_t iv_length() {
                return 16;
            }

            bool init(void const *keyData, void const *tweakData) {
                sha3_512 context{};
                context.init();
                context.update(keyData, 64);
                context.update(tweakData, 16);
                context.final(&hash_[0]);
                return true;
            }

            bool init(void const *keyData, std::size_t key_length) {
                hash_ = sha3_512_compute(keyData, key_length);
                return true;
            }

            bool init(std::vector<std::uint8_t> const &keyData) {
                hash_ = sha3_512_compute(keyData);
                return true;
            }

            bool init(std::string const &keyData) {
                hash_ = sha3_512_compute(keyData);
                return true;
            }

            void encrypt_block(void const *plain, void *c) const {
                for(std::size_t i = 0; i < 8; ++i) {
                    ((std::uint64_t *)c)[i] = ((std::uint64_t const *)plain)[i] ^ ((std::uint64_t const *)hash_.data())[i];
                }
            }

            void decrypt_block(void const *c, void *plain) const {
                encrypt_block(c, plain);
            }

        private:
            std::array<std::uint8_t, 64> hash_{};
        };

    }

}
