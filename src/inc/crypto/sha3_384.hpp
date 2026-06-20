#pragma once

#include "../commondefs.hpp"
#include "keccak.hpp"

namespace teal {

    namespace crypt {

        class sha3_384 final {
        public:
            static constexpr size_t digest_size() {
                return 384 / 8;
            }

            void init() {
                ctx_.init(2 * 384);
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

        static std::array<std::uint8_t, 48> sha3_384_compute(const void *data, size_t length) {
            sha3_384 context{};
            std::array<std::uint8_t, 48> res{};
            context.init();
            context.update(data, length);
            context.final(&res[0]);
            return res;
        }

    }

}
