#pragma once

#include "../commondefs.hpp"
#include "keccak.hpp"

namespace scfx {

    namespace crypt {

        class sha3_512 final {
        public:
            static constexpr size_t digest_size() {
                return 512 / 8;
            }

            void init() {
                ctx_.init(2 * 512);
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

        static std::array<std::uint8_t, 64> sha3_512_compute(const void *data, size_t length) {
            sha3_512 context{};
            std::array<std::uint8_t, 64> res{};
            context.init();
            context.update(data, length);
            context.final(&res[0]);
            return res;
        }

        static std::array<std::uint8_t, 64> sha3_512_compute(std::vector<std::uint8_t> const &data) {
            return sha3_512_compute(data.data(), data.size());
        }

        template<std::size_t N>
        std::array<std::uint8_t, 64> sha3_512_compute(std::array<std::uint8_t, N> const &data) {
            return sha3_512_compute(data.data(), data.size());
        }

        static std::array<std::uint8_t, 64> sha3_512_compute(std::string const &data) {
            return sha3_512_compute(data.data(), data.size());
        }

    }

}
