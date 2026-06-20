#pragma once

#include "../commondefs.hpp"
#include "../bit_util.hpp"

#define CRYPT_GCM_STORE32BE(a, p) \
   ((uint8_t *)(p))[0] = ((uint32_t)(a) >> 24) & 0xFFU, \
   ((uint8_t *)(p))[1] = ((uint32_t)(a) >> 16) & 0xFFU, \
   ((uint8_t *)(p))[2] = ((uint32_t)(a) >> 8) & 0xFFU, \
   ((uint8_t *)(p))[3] = ((uint32_t)(a) >> 0) & 0xFFU

#define CRYPT_GCM_STORE64BE(a, p) \
   ((uint8_t *)(p))[0] = ((uint64_t)(a) >> 56) & 0xFFU, \
   ((uint8_t *)(p))[1] = ((uint64_t)(a) >> 48) & 0xFFU, \
   ((uint8_t *)(p))[2] = ((uint64_t)(a) >> 40) & 0xFFU, \
   ((uint8_t *)(p))[3] = ((uint64_t)(a) >> 32) & 0xFFU, \
   ((uint8_t *)(p))[4] = ((uint64_t)(a) >> 24) & 0xFFU, \
   ((uint8_t *)(p))[5] = ((uint64_t)(a) >> 16) & 0xFFU, \
   ((uint8_t *)(p))[6] = ((uint64_t)(a) >> 8) & 0xFFU, \
   ((uint8_t *)(p))[7] = ((uint64_t)(a) >> 0) & 0xFFU

namespace teal {

    namespace crypt {

        template<typename CIPHER>
        struct gcm {
            gcm() = default;
            gcm(CIPHER *cipher_context_ptr) { init(cipher_context_ptr); }
            gcm(gcm const &) = default;
            gcm &operator=(gcm const &) = default;
            gcm(gcm &&) = default;
            gcm &operator=(gcm &&) = default;
            ~gcm() = default;

            bool init(CIPHER *cipher_context_ptr) {
                std::size_t i{};
                std::size_t j{};
                std::uint32_t c{};
                std::uint32_t h[4]{};

                if(CIPHER::block_size() != 16) {
                   return false;
                }

                cipher_context_ = cipher_context_ptr;

                h[0] = 0;
                h[1] = 0;
                h[2] = 0;
                h[3] = 0;

                cipher_context_->encrypt_block((uint8_t *) h, (uint8_t *) h);

                j = reverseInt4(0);
                m[j][0] = 0;
                m[j][1] = 0;
                m[j][2] = 0;
                m[j][3] = 0;

                j = reverseInt4(1);

                m[j][0] = teal::bit_util::hnswap<std::uint32_t>{h[3]}.val;
                m[j][1] = teal::bit_util::hnswap<std::uint32_t>{h[2]}.val;
                m[j][2] = teal::bit_util::hnswap<std::uint32_t>{h[1]}.val;
                m[j][3] = teal::bit_util::hnswap<std::uint32_t>{h[0]}.val;

                for(i = 2; i < 16; i++) {
                   if(i & 1) {
                      j = reverseInt4(i - 1);
                      h[0] = m[j][0];
                      h[1] = m[j][1];
                      h[2] = m[j][2];
                      h[3] = m[j][3];

                      j = reverseInt4(1);
                      h[0] ^= m[j][0];
                      h[1] ^= m[j][1];
                      h[2] ^= m[j][2];
                      h[3] ^= m[j][3];
                   } else {
                      j = reverseInt4(i / 2);
                      h[0] = m[j][0];
                      h[1] = m[j][1];
                      h[2] = m[j][2];
                      h[3] = m[j][3];

                      c = h[0] & 0x01;
                      h[0] = (h[0] >> 1) | (h[1] << 31);
                      h[1] = (h[1] >> 1) | (h[2] << 31);
                      h[2] = (h[2] >> 1) | (h[3] << 31);
                      h[3] >>= 1;

                      h[3] ^= red[reverseInt4(1)] & ~(c - 1);
                   }

                   j = reverseInt4(i);
                   m[j][0] = h[0];
                   m[j][1] = h[1];
                   m[j][2] = h[2];
                   m[j][3] = h[3];
                }

                return true;
            }

            bool encrypt(const uint8_t *iv, size_t ivLen, const uint8_t *a, size_t aLen, const uint8_t *p,
                         uint8_t *c, size_t length, uint8_t *t, size_t tLen) {
                 size_t k;
                 size_t n;
                 uint8_t b[16];
                 uint8_t j[16];
                 uint8_t s[16];

                 if(ivLen < 1) {
                    return false;
                 }

                 if(tLen < 4 || tLen > 16) {
                    return false;
                 }

                 if(ivLen == 12) {
                    std::memcpy(j, iv, 12);
                    CRYPT_GCM_STORE32BE(1, j + 12);
                 } else {
                    std::memset(j, 0, 16);

                    n = ivLen;

                    while(n > 0) {
                       k = std::min<decltype(n)>(n, 16);

                       gcmXorBlock(j, j, iv, k);
                       gcmMul(j);

                       iv += k;
                       n -= k;
                    }

                    std::memset(b, 0, 8);
                    CRYPT_GCM_STORE64BE(ivLen * 8, b + 8);

                    gcmXorBlock(j, j, b, 16);
                    gcmMul(j);
                 }

                 cipher_context_->encrypt_block(j, b);
                 std::memcpy(t, b, tLen);

                 std::memset(s, 0, 16);
                 n = aLen;

                 while(n > 0) {
                    k = std::min<decltype(n)>(n, 16);

                    gcmXorBlock(s, s, a, k);
                    gcmMul(s);

                    a += k;
                    n -= k;
                 }

                 n = length;

                 while(n > 0) {
                    k = std::min<decltype(n)>(n, 16);

                    gcmIncCounter(j);

                    cipher_context_->encrypt_block(j, b);
                    gcmXorBlock(c, p, b, k);

                    gcmXorBlock(s, s, c, k);
                    gcmMul(s);

                    p += k;
                    c += k;
                    n -= k;
                 }

                 CRYPT_GCM_STORE64BE(aLen * 8, b);
                 CRYPT_GCM_STORE64BE(length * 8, b + 8);

                 gcmXorBlock(s, s, b, 16);
                 gcmMul(s);

                 gcmXorBlock(t, t, s, tLen);

                 return true;
            }

            bool decrypt(const uint8_t *iv, size_t ivLen, const uint8_t *a, size_t aLen, const uint8_t *c,
                         uint8_t *p, size_t length, const uint8_t *t, size_t tLen) {
                uint8_t mask;
                size_t k;
                size_t n;
                uint8_t b[16];
                uint8_t j[16];
                uint8_t r[16];
                uint8_t s[16];

                if(ivLen < 1)
                   return false;

                if(tLen < 4 || tLen > 16)
                   return false;

                if(ivLen == 12) {
                   std::memcpy(j, iv, 12);
                   CRYPT_GCM_STORE32BE(1, j + 12);
                } else {
                   std::memset(j, 0, 16);

                   n = ivLen;

                   while(n > 0) {
                      k = std::min<decltype(n)>(n, 16);

                      gcmXorBlock(j, j, iv, k);
                      gcmMul(j);

                      iv += k;
                      n -= k;
                   }

                   std::memset(b, 0, 8);
                   CRYPT_GCM_STORE64BE(ivLen * 8, b + 8);

                   gcmXorBlock(j, j, b, 16);
                   gcmMul(j);
                }

                cipher_context_->encrypt_block(j, b);
                std::memcpy(r, b, tLen);

                std::memset(s, 0, 16);
                n = aLen;

                while(n > 0) {
                   k = std::min<decltype(n)>(n, 16);

                   gcmXorBlock(s, s, a, k);
                   gcmMul(s);

                   a += k;
                   n -= k;
                }

                n = length;

                while(n > 0) {
                   k = std::min<decltype(n)>(n, 16);

                   gcmXorBlock(s, s, c, k);
                   gcmMul(s);

                   gcmIncCounter(j);

                   cipher_context_->encrypt_block(j, b);
                   gcmXorBlock(p, c, b, k);

                   c += k;
                   p += k;
                   n -= k;
                }

                CRYPT_GCM_STORE64BE(aLen * 8, b);
                CRYPT_GCM_STORE64BE(length * 8, b + 8);

                gcmXorBlock(s, s, b, 16);
                gcmMul(s);

                gcmXorBlock(r, r, s, tLen);

                for(mask = 0, n = 0; n < tLen; n++) {
                   mask |= r[n] ^ t[n];
                }

                return mask == 0;
            }

        private:
            static const inline std::uint32_t red[16] = {
               0x00000000,
               0x1C200000,
               0x38400000,
               0x24600000,
               0x70800000,
               0x6CA00000,
               0x48C00000,
               0x54E00000,
               0xE1000000,
               0xFD200000,
               0xD9400000,
               0xC5600000,
               0x91800000,
               0x8DA00000,
               0xA9C00000,
               0xB5E00000
            };

            uint8_t reverseInt4(std::uint8_t value) {
               value = ((value & 0x0C) >> 2) | ((value & 0x03) << 2);
               value = ((value & 0x0A) >> 1) | ((value & 0x05) << 1);

               return value;
            }

            void gcmMul(uint8_t *x) {
               int i;
               uint8_t b;
               uint8_t c;
               uint32_t z[4];

               z[0] = 0;
               z[1] = 0;
               z[2] = 0;
               z[3] = 0;

               for(i = 15; i >= 0; i--) {
                  b = x[i] & 0x0F;

                  c = z[0] & 0x0F;
                  z[0] = (z[0] >> 4) | (z[1] << 28);
                  z[1] = (z[1] >> 4) | (z[2] << 28);
                  z[2] = (z[2] >> 4) | (z[3] << 28);
                  z[3] >>= 4;

                  z[0] ^= m[b][0];
                  z[1] ^= m[b][1];
                  z[2] ^= m[b][2];
                  z[3] ^= m[b][3];

                  z[3] ^= red[c];

                  b = (x[i] >> 4) & 0x0F;

                  c = z[0] & 0x0F;
                  z[0] = (z[0] >> 4) | (z[1] << 28);
                  z[1] = (z[1] >> 4) | (z[2] << 28);
                  z[2] = (z[2] >> 4) | (z[3] << 28);
                  z[3] >>= 4;

                  z[0] ^= m[b][0];
                  z[1] ^= m[b][1];
                  z[2] ^= m[b][2];
                  z[3] ^= m[b][3];

                  z[3] ^= red[c];
               }

               CRYPT_GCM_STORE32BE(z[3], x);
               CRYPT_GCM_STORE32BE(z[2], x + 4);
               CRYPT_GCM_STORE32BE(z[1], x + 8);
               CRYPT_GCM_STORE32BE(z[0], x + 12);
            }

            void gcmXorBlock(uint8_t *x, const uint8_t *a, const uint8_t *b, size_t n) {
               size_t i;

               for(i = 0; i < n; i++) {
                  x[i] = a[i] ^ b[i];
               }
            }

            void gcmIncCounter(uint8_t *x) {
               uint16_t temp;

               temp = x[15] + 1;
               x[15] = temp & 0xFF;
               temp = (temp >> 8) + x[14];
               x[14] = temp & 0xFF;
               temp = (temp >> 8) + x[13];
               x[13] = temp & 0xFF;
               temp = (temp >> 8) + x[12];
               x[12] = temp & 0xFF;
            }

        private:
            CIPHER *cipher_context_{nullptr};
            std::uint32_t m[16][4];
        };

    }

}

#undef CRYPT_GCM_STORE32BE
#undef CRYPT_GCM_STORE64BE
