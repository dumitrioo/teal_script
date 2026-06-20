#pragma once

#include "../commondefs.hpp"
#include "keccak.hpp"

namespace teal {

    namespace crypt {

        class sha3_256 final {
        public:
            static constexpr size_t digest_size() {
                return 256 / 8;
            }

            void init() {
                ctx_.init(2 * 256);
            }

            void update(const void *data, size_t length) {
                ctx_.absorb(data, length);
            }

            void final(uint8_t *digest) {
                ctx_.final(keccak::sha3_pad());
                ctx_.squeeze(digest, digest_size());
            }

        private:
            keccak ctx_{};
        };

        static std::array<std::uint8_t, 32> sha3_256_compute(const void *data, size_t length) {
            sha3_256 context{};
            std::array<std::uint8_t, 32> res{};
            context.init();
            context.update(data, length);
            context.final(&res[0]);
            return res;
        }

        static std::array<std::uint8_t, 32> sha3_256_compute(std::string const &data) {
            return sha3_256_compute(data.data(), data.size());
        }

        static std::array<std::uint8_t, 32> sha3_256_compute(std::vector<std::uint8_t> const &data) {
            return sha3_256_compute(data.data(), data.size());
        }

        template<std::size_t N>
        std::array<std::uint8_t, 32> sha3_256_compute(std::array<std::uint8_t, N> const &data) {
            return sha3_256_compute(data.data(), data.size());
        }

    }

}
