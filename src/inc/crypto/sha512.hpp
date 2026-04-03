#pragma once

#include "../commondefs.hpp"
#include "../bit_util.hpp"

namespace teal {

    namespace crypt {

        class sha512 {
        public:
            sha512() {
                reset();
            }

            void reset() {
                h_[0] = 0x6A09E667F3BCC908ULL;
                h_[1] = 0xBB67AE8584CAA73BULL;
                h_[2] = 0x3C6EF372FE94F82BULL;
                h_[3] = 0xA54FF53A5F1D36F1ULL;
                h_[4] = 0x510E527FADE682D1ULL;
                h_[5] = 0x9B05688C2B3E6C1FULL;
                h_[6] = 0x1F83D9ABFB41BD6BULL;
                h_[7] = 0x5BE0CD19137E2179ULL;

                size_ = 0;
                totalSize_ = 0;
            }

            void compute(const void *data, std::size_t length, std::uint8_t *digest) {
                reset();
                update(data, length);
                final(digest);
            }

            static constexpr size_t digest_size() {
                return 64;
            }

        private:
            void update(const void *data, std::size_t length) {
                while(length > 0) {
                    std::size_t n{std::min(length, 128 - size_)};

                    std::memcpy(buffer_ + size_, data, n);

                    size_ += n;
                    totalSize_ += n;
                    data = (uint8_t const *)data + n;
                    length -= n;

                    if(size_ == 128) {
                        process_block();
                        size_ = 0;
                    }
                }
            }

            void final(uint8_t *digest) {
                uint32_t i;
                size_t paddingSize;
                uint64_t totalSize;

                totalSize = totalSize_ * 8;

                if(size_ < 112) {
                   paddingSize = 112 - size_;
                } else {
                   paddingSize = 128 + 112 - size_;
                }

                update(padding, paddingSize);

                w_[14] = 0;
                w_[15] = bit_util::hnswap<std::uint64_t>{totalSize}.val;

                process_block();

                for(i = 0; i < 8; i++) {
                    h_[i] = bit_util::hnswap<std::uint64_t>{h_[i]}.val;
                }

                if(digest) {
                    std::memcpy(digest, digest_, digest_size());
                }
            }

            void process_block() {
                uint32_t t;
                uint64_t temp1;
                uint64_t temp2;

                uint64_t a = h_[0];
                uint64_t b = h_[1];
                uint64_t c = h_[2];
                uint64_t d = h_[3];
                uint64_t e = h_[4];
                uint64_t f = h_[5];
                uint64_t g = h_[6];
                uint64_t h = h_[7];

                uint64_t *w = w_;

                for(t = 0; t < 16; t++) {
                    w[t] = bit_util::hnswap<std::uint64_t>{w[t]}.val;
                }

                for(t = 0; t < 80; t++) {
                    if(t >= 16) {
                        W(w, t) += SIGMA4(W(w, t + 14)) + W(w, t + 9) + SIGMA3(W(w, t + 1));
                    }

                    temp1 = h + SIGMA2(e) + CH(e, f, g) + k[t] + W(w, t);
                    temp2 = SIGMA1(a) + MAJ(a, b, c);

                    h = g;
                    g = f;
                    f = e;
                    e = d + temp1;
                    d = c;
                    c = b;
                    b = a;
                    a = temp1 + temp2;
                }

                h_[0] += a;
                h_[1] += b;
                h_[2] += c;
                h_[3] += d;
                h_[4] += e;
                h_[5] += f;
                h_[6] += g;
                h_[7] += h;
            }

            std::uint64_t CH(std::uint64_t x, std::uint64_t y, std::uint64_t z) {
                return (((x) & (y)) | (~(x) & (z)));
            }

            std::uint64_t MAJ(std::uint64_t x, std::uint64_t y, std::uint64_t z) {
                return (((x) & (y)) | ((x) & (z)) | ((y) & (z)));
            }

            std::uint64_t SIGMA1(std::uint64_t x) {
                return bit_util::ror<std::uint64_t>(x, 28).val ^ bit_util::ror<std::uint64_t>(x, 34).val ^ bit_util::ror<std::uint64_t>(x, 39).val;
            }

            std::uint64_t SIGMA2(std::uint64_t x) {
                return bit_util::ror<std::uint64_t>(x, 14).val ^ bit_util::ror<std::uint64_t>(x, 18).val ^ bit_util::ror<std::uint64_t>(x, 41).val;
            }

            std::uint64_t SIGMA3(std::uint64_t x) {
                return bit_util::ror<std::uint64_t>(x, 1).val ^ bit_util::ror<std::uint64_t>(x, 8).val ^ (x >> 7);
            }

            std::uint64_t SIGMA4(std::uint64_t x) {
                return bit_util::ror<std::uint64_t>(x, 19).val ^ bit_util::ror<std::uint64_t>(x, 61).val ^ (x >> 6);
            }

            std::uint64_t &W(uint64_t *w, std::uint64_t t) {
                return w[t & 0x0FULL];
            }

            static inline const uint8_t padding[128] {
                0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };

            static inline const uint64_t k[80] {
                0x428A2F98D728AE22ULL, 0x7137449123EF65CDULL, 0xB5C0FBCFEC4D3B2FULL, 0xE9B5DBA58189DBBCULL,
                0x3956C25BF348B538ULL, 0x59F111F1B605D019ULL, 0x923F82A4AF194F9BULL, 0xAB1C5ED5DA6D8118ULL,
                0xD807AA98A3030242ULL, 0x12835B0145706FBEULL, 0x243185BE4EE4B28CULL, 0x550C7DC3D5FFB4E2ULL,
                0x72BE5D74F27B896FULL, 0x80DEB1FE3B1696B1ULL, 0x9BDC06A725C71235ULL, 0xC19BF174CF692694ULL,
                0xE49B69C19EF14AD2ULL, 0xEFBE4786384F25E3ULL, 0x0FC19DC68B8CD5B5ULL, 0x240CA1CC77AC9C65ULL,
                0x2DE92C6F592B0275ULL, 0x4A7484AA6EA6E483ULL, 0x5CB0A9DCBD41FBD4ULL, 0x76F988DA831153B5ULL,
                0x983E5152EE66DFABULL, 0xA831C66D2DB43210ULL, 0xB00327C898FB213FULL, 0xBF597FC7BEEF0EE4ULL,
                0xC6E00BF33DA88FC2ULL, 0xD5A79147930AA725ULL, 0x06CA6351E003826FULL, 0x142929670A0E6E70ULL,
                0x27B70A8546D22FFCULL, 0x2E1B21385C26C926ULL, 0x4D2C6DFC5AC42AEDULL, 0x53380D139D95B3DFULL,
                0x650A73548BAF63DEULL, 0x766A0ABB3C77B2A8ULL, 0x81C2C92E47EDAEE6ULL, 0x92722C851482353BULL,
                0xA2BFE8A14CF10364ULL, 0xA81A664BBC423001ULL, 0xC24B8B70D0F89791ULL, 0xC76C51A30654BE30ULL,
                0xD192E819D6EF5218ULL, 0xD69906245565A910ULL, 0xF40E35855771202AULL, 0x106AA07032BBD1B8ULL,
                0x19A4C116B8D2D0C8ULL, 0x1E376C085141AB53ULL, 0x2748774CDF8EEB99ULL, 0x34B0BCB5E19B48A8ULL,
                0x391C0CB3C5C95A63ULL, 0x4ED8AA4AE3418ACBULL, 0x5B9CCA4F7763E373ULL, 0x682E6FF3D6B2B8A3ULL,
                0x748F82EE5DEFB2FCULL, 0x78A5636F43172F60ULL, 0x84C87814A1F0AB72ULL, 0x8CC702081A6439ECULL,
                0x90BEFFFA23631E28ULL, 0xA4506CEBDE82BDE9ULL, 0xBEF9A3F7B2C67915ULL, 0xC67178F2E372532BULL,
                0xCA273ECEEA26619CULL, 0xD186B8C721C0C207ULL, 0xEADA7DD6CDE0EB1EULL, 0xF57D4F7FEE6ED178ULL,
                0x06F067AA72176FBAULL, 0x0A637DC5A2C898A6ULL, 0x113F9804BEF90DAEULL, 0x1B710B35131C471BULL,
                0x28DB77F523047D84ULL, 0x32CAAB7B40C72493ULL, 0x3C9EBE0A15C9BEBCULL, 0x431D67C49C100D4CULL,
                0x4CC5D4BECB3E42B6ULL, 0x597F299CFC657E2AULL, 0x5FCB6FAB3AD6FAECULL, 0x6C44198C4A475817ULL
            };

        private:
            union {
                uint64_t h_[8];
                uint8_t digest_[64]{};
            };
            union {
                uint64_t w_[16];
                uint8_t buffer_[128]{};
            };
            size_t size_{};
            uint64_t totalSize_{};
        };

        static std::array<std::uint8_t, sha512::digest_size()> sha512sum(void const *input, std::size_t insize) {
            std::array<std::uint8_t, sha512::digest_size()> res{};
            sha512 ctx{};
            ctx.compute(input, insize, res.data());
            return res;
        }

        static std::string sha512sum(std::string const &input) {
            std::array<std::uint8_t, sha512::digest_size()> res{sha512sum(input.data(), input.size())};
            char buf[2 * sha512::digest_size() + 1];
            for(std::size_t i = 0; i < sha512::digest_size(); i++) {
                sprintf(buf + i * 2, "%02x", res[i]);
            }
            buf[2 * sha512::digest_size()] = 0;
            return buf;
        }

    }

}
