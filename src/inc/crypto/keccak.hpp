#pragma once

#include "../commondefs.hpp"
#include "../bit_util.hpp"

//The binary logarithm of the lane size
#ifndef KECCAK_L
   #define KECCAK_L 6
#endif

//Check lane size
#if (KECCAK_L == 3)
   //Base type that represents a lane
   typedef std::uint8_t keccak_lane_t;
   //Rotate left operation
   #define KECCAK_ROL(a, n) ROL8(a, n)
   //Host byte order to little-endian byte order
   #define KECCAK_HTOLE(a) (a)
   //Little-endian byte order to host byte order
   #define KECCAK_LETOH(a) (a)
#elif (KECCAK_L == 4)
   //Base type that represents a lane
   #define keccak_lane_t std::uint16_t
   //Rotate left operation
   #define KECCAK_ROL(a, n) bit_util::rol<std::uint16_t>{(std::uint16_t)a, n}.val
   //Host byte order to little-endian byte order
   #define KECCAK_HTOLE(a) bit_util::swap_on_be<std::uint16_t>{(std::uint16_t)a}.val
   //Little-endian byte order to host byte order
   #define KECCAK_LETOH(a) bit_util::swap_on_be<std::uint16_t>{(std::uint16_t)a}.val
#elif (KECCAK_L == 5)
   //Base type that represents a lane
   #define keccak_lane_t std::uint32_t
   //Rotate left operation
   #define KECCAK_ROL(a, n) bit_util::rol<std::uint32_t>{(std::uint32_t)a, n}.val
   //Host byte order to little-endian byte order
   #define KECCAK_HTOLE(a) bit_util::swap_on_be<std::uint32_t>{(std::uint32_t)a}.val
   //Little-endian byte order to host byte order
   #define KECCAK_LETOH(a) bit_util::swap_on_be<std::uint32_t>{(std::uint32_t)a}.val
#elif (KECCAK_L == 6)
   //Base type that represents a lane
   #define keccak_lane_t std::uint64_t
   //Rotate left operation
   #define KECCAK_ROL(a, n) bit_util::rol<std::uint64_t>{(std::uint64_t)a, n}.val
   //Host byte order to little-endian byte order
   #define KECCAK_HTOLE(a) bit_util::swap_on_be<std::uint64_t>{(std::uint64_t)a}.val
   //Little-endian byte order to host byte order
   #define KECCAK_LETOH(a) bit_util::swap_on_be<std::uint64_t>{(std::uint64_t)a}.val
#else
   #error KECCAK_L parameter is not valid
#endif

//The lane size of a Keccak-p permutation in bits
#define KECCAK_W (1 << KECCAK_L)
//The width of a Keccak-p permutation
#define KECCAK_B (KECCAK_W * 25)
//The number of rounds for a Keccak-p permutation
#define KECCAK_NR (12 + 2 * KECCAK_L)

//Keccak padding byte
#define KECCAK_PAD 0x01
//SHA-3 padding byte
#define KECCAK_SHA3_PAD 0x06
//SHAKE padding byte
#define KECCAK_SHAKE_PAD 0x1F
//cSHAKE padding byte
#define KECCAK_CSHAKE_PAD 0x04

namespace teal {

    namespace crypt {

        class keccak final {
        public:
            static constexpr std::size_t pad() {
                return KECCAK_PAD;
            }

            static constexpr std::size_t sha3_pad() {
                return KECCAK_SHA3_PAD;
            }

            static constexpr std::size_t shake_pad() {
                return KECCAK_SHAKE_PAD;
            }

            static constexpr std::size_t cshake_pad() {
                return KECCAK_CSHAKE_PAD;
            }

            bool init(unsigned int capacity) {
                unsigned int rate;
                a[0][0] = 0; a[0][1] = 0; a[0][2] = 0; a[0][3] = 0; a[0][4] = 0;
                a[1][0] = 0; a[1][1] = 0; a[1][2] = 0; a[1][3] = 0; a[1][4] = 0;
                a[2][0] = 0; a[2][1] = 0; a[2][2] = 0; a[2][3] = 0; a[2][4] = 0;
                a[3][0] = 0; a[3][1] = 0; a[3][2] = 0; a[3][3] = 0; a[3][4] = 0;
                a[4][0] = 0; a[4][1] = 0; a[4][2] = 0; a[4][3] = 0; a[4][4] = 0;
                block[0] = 0;  block[1] = 0;  block[2] = 0;  block[3] = 0;  block[4] = 0;  block[5] = 0;
                block[6] = 0;  block[7] = 0;  block[8] = 0;  block[9] = 0;  block[10] = 0; block[11] = 0;
                block[12] = 0; block[13] = 0; block[14] = 0; block[15] = 0; block[16] = 0; block[17] = 0;
                block[18] = 0; block[19] = 0; block[20] = 0; block[21] = 0; block[22] = 0; block[23] = 0;
                length = 0;

                if(capacity >= KECCAK_B) {
                   return false;
                }

                rate = KECCAK_B - capacity;

                if((rate % KECCAK_W) != 0) {
                   return false;
                }

                blockSize = rate / 8;

                return true;
            }

            void absorb(const void *input, size_t length) {
                unsigned int i;
                size_t n;
                keccak_lane_t *a;

                //Point to the state array
                a = (keccak_lane_t *) this->a;

                //Absorbing phase
                while(length > 0) {
                   //Limit the number of bytes to process at a time
                   n = std::min(length, this->blockSize - this->length);

                   //Copy the data to the buffer
                   std::memcpy(this->buffer + this->length, input, n);

                   //Number of data bytes that have been buffered
                   this->length += n;

                   //Advance the data pointer
                   input = (uint8_t *) input + n;
                   //Remaining bytes to process
                   length -= n;

                   //Absorb the message block by block
                   if(this->length == this->blockSize) {
                      //Absorb the current block
                      for(i = 0; i < this->blockSize / sizeof(keccak_lane_t); i++) {
                         a[i] ^= KECCAK_LETOH(this->block[i]);
                      }

                      //Apply block permutation function
                      keccakPermutBlock();

                      //The input buffer is empty
                      this->length = 0;
                   }
                }
            }

            void final(uint8_t pad) {
                unsigned int i;
                size_t q;
                keccak_lane_t *a;

                //Point to the state array
                a = (keccak_lane_t *) this->a;

                //Compute the number of padding bytes
                q = this->blockSize - this->length;

                //Append padding
                std::memset(this->buffer + this->length, 0, q);
                this->buffer[this->length] |= pad;
                this->buffer[this->blockSize - 1] |= 0x80;

                //Absorb the final block
                for(i = 0; i < this->blockSize / sizeof(keccak_lane_t); i++) {
                   a[i] ^= KECCAK_LETOH(this->block[i]);
                }

                //Apply block permutation function
                keccakPermutBlock();

                //Convert lanes to little-endian byte order
                for(i = 0; i < this->blockSize / sizeof(keccak_lane_t); i++) {
                   a[i] = KECCAK_HTOLE(a[i]);
                }

                //Number of bytes available in the output buffer
                this->length = this->blockSize;
            }

            void squeeze(uint8_t *output, size_t length) {
                unsigned int i;
                size_t n;
                keccak_lane_t *a;

                //Point to the state array
                a = (keccak_lane_t *) this->a;

                //An arbitrary number of output bits can be squeezed out of the state
                while(length > 0) {
                   //Check whether more data is required
                   if(this->length == 0) {
                      //Convert lanes to host byte order
                      for(i = 0; i < this->blockSize / sizeof(keccak_lane_t); i++) {
                         a[i] = KECCAK_LETOH(a[i]);
                      }

                      //Apply block permutation function
                      keccakPermutBlock();

                      //Convert lanes to little-endian byte order
                      for(i = 0; i < this->blockSize / sizeof(keccak_lane_t); i++) {
                         a[i] = KECCAK_HTOLE(a[i]);
                      }

                      //Number of bytes available in the output buffer
                      this->length = this->blockSize;
                   }

                   //Compute the number of bytes to process at a time
                   n = std::min(length, this->length);

                   //Copy the output string
                   if(output != NULL) {
                      std::memcpy(output, this->digest + this->blockSize -
                         this->length, n);
                   }

                   //Number of bytes available in the output buffer
                   this->length -= n;

                   //Advance the data pointer
                   output = (uint8_t *) output + n;
                   //Number of bytes that remains to be written
                   length -= n;
                }
            }

        private:
            void keccakPermutBlock() {
                unsigned int i;

                //Each round consists of a sequence of five transformations,
                //which are called the step mappings
                for(i = 0; i < KECCAK_NR; i++) {
                   //Apply theta step mapping
                   theta(a);
                   //Apply rho step mapping
                   rho(a);
                   //Apply pi step mapping
                   pi(a);
                   //Apply chi step mapping
                   chi(a);
                   //Apply iota step mapping
                   iota(a, i);
                }
            }

        private:
            static constexpr keccak_lane_t rc[KECCAK_NR] = {
               (keccak_lane_t) 0x0000000000000001,
               (keccak_lane_t) 0x0000000000008082,
               (keccak_lane_t) 0x800000000000808A,
               (keccak_lane_t) 0x8000000080008000,
               (keccak_lane_t) 0x000000000000808B,
               (keccak_lane_t) 0x0000000080000001,
               (keccak_lane_t) 0x8000000080008081,
               (keccak_lane_t) 0x8000000000008009,
               (keccak_lane_t) 0x000000000000008A,
               (keccak_lane_t) 0x0000000000000088,
               (keccak_lane_t) 0x0000000080008009,
               (keccak_lane_t) 0x000000008000000A,
               (keccak_lane_t) 0x000000008000808B,
               (keccak_lane_t) 0x800000000000008B,
               (keccak_lane_t) 0x8000000000008089,
               (keccak_lane_t) 0x8000000000008003,
               (keccak_lane_t) 0x8000000000008002,
               (keccak_lane_t) 0x8000000000000080,
            #if (KECCAK_L >= 4)
               (keccak_lane_t) 0x000000000000800A,
               (keccak_lane_t) 0x800000008000000A,
            #endif
            #if (KECCAK_L >= 5)
               (keccak_lane_t) 0x8000000080008081,
               (keccak_lane_t) 0x8000000000008080,
            #endif
            #if (KECCAK_L >= 6)
               (keccak_lane_t) 0x0000000080000001,
               (keccak_lane_t) 0x8000000080008008
            #endif
            };

            static void theta(keccak_lane_t a[5][5]) {
               keccak_lane_t c[5];
               keccak_lane_t d[5];

               //The effect of the theta transformation is to XOR each bit in the
               //state with the parities of two columns in the array
               c[0] = a[0][0] ^ a[1][0] ^ a[2][0] ^ a[3][0] ^ a[4][0];
               c[1] = a[0][1] ^ a[1][1] ^ a[2][1] ^ a[3][1] ^ a[4][1];
               c[2] = a[0][2] ^ a[1][2] ^ a[2][2] ^ a[3][2] ^ a[4][2];
               c[3] = a[0][3] ^ a[1][3] ^ a[2][3] ^ a[3][3] ^ a[4][3];
               c[4] = a[0][4] ^ a[1][4] ^ a[2][4] ^ a[3][4] ^ a[4][4];

               d[0] = c[4] ^ KECCAK_ROL(c[1], 1);
               d[1] = c[0] ^ KECCAK_ROL(c[2], 1);
               d[2] = c[1] ^ KECCAK_ROL(c[3], 1);
               d[3] = c[2] ^ KECCAK_ROL(c[4], 1);
               d[4] = c[3] ^ KECCAK_ROL(c[0], 1);

               a[0][0] ^= d[0];
               a[1][0] ^= d[0];
               a[2][0] ^= d[0];
               a[3][0] ^= d[0];
               a[4][0] ^= d[0];

               a[0][1] ^= d[1];
               a[1][1] ^= d[1];
               a[2][1] ^= d[1];
               a[3][1] ^= d[1];
               a[4][1] ^= d[1];

               a[0][2] ^= d[2];
               a[1][2] ^= d[2];
               a[2][2] ^= d[2];
               a[3][2] ^= d[2];
               a[4][2] ^= d[2];

               a[0][3] ^= d[3];
               a[1][3] ^= d[3];
               a[2][3] ^= d[3];
               a[3][3] ^= d[3];
               a[4][3] ^= d[3];

               a[0][4] ^= d[4];
               a[1][4] ^= d[4];
               a[2][4] ^= d[4];
               a[3][4] ^= d[4];
               a[4][4] ^= d[4];
            }

            static void rho(keccak_lane_t a[5][5]) {
               //The effect of the rho transformation is to rotate the bits of each lane by
               //an offset, which depends on the fixed x and y coordinates of the lane
               a[0][1] = KECCAK_ROL(a[0][1], 1   % KECCAK_W);
               a[0][2] = KECCAK_ROL(a[0][2], 190 % KECCAK_W);
               a[0][3] = KECCAK_ROL(a[0][3], 28  % KECCAK_W);
               a[0][4] = KECCAK_ROL(a[0][4], 91  % KECCAK_W);

               a[1][0] = KECCAK_ROL(a[1][0], 36  % KECCAK_W);
               a[1][1] = KECCAK_ROL(a[1][1], 300 % KECCAK_W);
               a[1][2] = KECCAK_ROL(a[1][2], 6   % KECCAK_W);
               a[1][3] = KECCAK_ROL(a[1][3], 55  % KECCAK_W);
               a[1][4] = KECCAK_ROL(a[1][4], 276 % KECCAK_W);

               a[2][0] = KECCAK_ROL(a[2][0], 3   % KECCAK_W);
               a[2][1] = KECCAK_ROL(a[2][1], 10  % KECCAK_W);
               a[2][2] = KECCAK_ROL(a[2][2], 171 % KECCAK_W);
               a[2][3] = KECCAK_ROL(a[2][3], 153 % KECCAK_W);
               a[2][4] = KECCAK_ROL(a[2][4], 231 % KECCAK_W);

               a[3][0] = KECCAK_ROL(a[3][0], 105 % KECCAK_W);
               a[3][1] = KECCAK_ROL(a[3][1], 45  % KECCAK_W);
               a[3][2] = KECCAK_ROL(a[3][2], 15  % KECCAK_W);
               a[3][3] = KECCAK_ROL(a[3][3], 21  % KECCAK_W);
               a[3][4] = KECCAK_ROL(a[3][4], 136 % KECCAK_W);

               a[4][0] = KECCAK_ROL(a[4][0], 210 % KECCAK_W);
               a[4][1] = KECCAK_ROL(a[4][1], 66  % KECCAK_W);
               a[4][2] = KECCAK_ROL(a[4][2], 253 % KECCAK_W);
               a[4][3] = KECCAK_ROL(a[4][3], 120 % KECCAK_W);
               a[4][4] = KECCAK_ROL(a[4][4], 78  % KECCAK_W);
            }

            static void pi(keccak_lane_t a[5][5]) {
               keccak_lane_t temp;

               //The effect of the pi transformation is to rearrange the
               //positions of the lanes
               temp = a[0][1];
               a[0][1] = a[1][1];
               a[1][1] = a[1][4];
               a[1][4] = a[4][2];
               a[4][2] = a[2][4];
               a[2][4] = a[4][0];
               a[4][0] = a[0][2];
               a[0][2] = a[2][2];
               a[2][2] = a[2][3];
               a[2][3] = a[3][4];
               a[3][4] = a[4][3];
               a[4][3] = a[3][0];
               a[3][0] = a[0][4];
               a[0][4] = a[4][4];
               a[4][4] = a[4][1];
               a[4][1] = a[1][3];
               a[1][3] = a[3][1];
               a[3][1] = a[1][0];
               a[1][0] = a[0][3];
               a[0][3] = a[3][3];
               a[3][3] = a[3][2];
               a[3][2] = a[2][1];
               a[2][1] = a[1][2];
               a[1][2] = a[2][0];
               a[2][0] = temp;
            }

            static void chi(keccak_lane_t a[5][5]) {
               keccak_lane_t temp1;
               keccak_lane_t temp2;

               //The effect of the chi transformation is to XOR each bit with
               //a non linear function of two other bits in its row
               temp1 = a[0][0];
               temp2 = a[0][1];
               a[0][0] ^= ~a[0][1] & a[0][2];
               a[0][1] ^= ~a[0][2] & a[0][3];
               a[0][2] ^= ~a[0][3] & a[0][4];
               a[0][3] ^= ~a[0][4] & temp1;
               a[0][4] ^= ~temp1 & temp2;

               temp1 = a[1][0];
               temp2 = a[1][1];
               a[1][0] ^= ~a[1][1] & a[1][2];
               a[1][1] ^= ~a[1][2] & a[1][3];
               a[1][2] ^= ~a[1][3] & a[1][4];
               a[1][3] ^= ~a[1][4] & temp1;
               a[1][4] ^= ~temp1 & temp2;

               temp1 = a[2][0];
               temp2 = a[2][1];
               a[2][0] ^= ~a[2][1] & a[2][2];
               a[2][1] ^= ~a[2][2] & a[2][3];
               a[2][2] ^= ~a[2][3] & a[2][4];
               a[2][3] ^= ~a[2][4] & temp1;
               a[2][4] ^= ~temp1 & temp2;

               temp1 = a[3][0];
               temp2 = a[3][1];
               a[3][0] ^= ~a[3][1] & a[3][2];
               a[3][1] ^= ~a[3][2] & a[3][3];
               a[3][2] ^= ~a[3][3] & a[3][4];
               a[3][3] ^= ~a[3][4] & temp1;
               a[3][4] ^= ~temp1 & temp2;

               temp1 = a[4][0];
               temp2 = a[4][1];
               a[4][0] ^= ~a[4][1] & a[4][2];
               a[4][1] ^= ~a[4][2] & a[4][3];
               a[4][2] ^= ~a[4][3] & a[4][4];
               a[4][3] ^= ~a[4][4] & temp1;
               a[4][4] ^= ~temp1 & temp2;
            }

            static void iota(keccak_lane_t a[5][5], unsigned int index) {
               //The iota transformation is parameterized by the round index
               a[0][0] ^= rc[index];
            }

            union {
              keccak_lane_t a[5][5];
              uint8_t digest[1];
            };
            union {
              keccak_lane_t block[24];
              uint8_t buffer[1];
            };
            unsigned int blockSize;
            size_t length;

        };

    }

}

#ifdef KECCAK_L
#undef KECCAK_L
#endif

#ifdef keccak_lane_t
#undef keccak_lane_t
#endif

#undef KECCAK_ROL
#undef KECCAK_HTOLE
#undef KECCAK_LETOH

#undef KECCAK_W
#undef KECCAK_B
#undef KECCAK_NR

#undef KECCAK_PAD
#undef KECCAK_SHA3_PAD
#undef KECCAK_SHAKE_PAD
#undef KECCAK_CSHAKE_PAD






