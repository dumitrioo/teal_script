#pragma once

#include "../commondefs.hpp"
#include "../bit_util.hpp"

namespace teal {

    namespace crypt {

        namespace detail {

            static uint32_t LOAD32LE(void const *p) {
               return (((uint32_t)(((uint8_t *)(p))[0]) << 0) |
                       ((uint32_t)(((uint8_t *)(p))[1]) << 8) |
                       ((uint32_t)(((uint8_t *)(p))[2]) << 16) |
                       ((uint32_t)(((uint8_t *)(p))[3]) << 24));
            }

            static void STORE32LE(uint32_t a, void *p) {
                uint8_t *bp{(uint8_t *)p};
                bp[0] = (a >> 0) & 0xffU;
                bp[1] = (a >> 8) & 0xffU;
                bp[2] = (a >> 16) & 0xffU;
                bp[3] = (a >> 24) & 0xffU;
            }
        }

        class poly1305 {
        public:
            poly1305() = default;

            poly1305(const uint8_t *key) {
                init(key);
            }

            poly1305(poly1305 const &) = default;
            poly1305 &operator=(poly1305 const &) = default;
            poly1305(poly1305 &&) = default;
            poly1305 &operator=(poly1305 &&) = default;
            ~poly1305() = default;

            void init(const uint8_t *key) {
                //The 256-bit key is partitioned into two parts, called r and s
                r[0] = detail::LOAD32LE(key);
                r[1] = detail::LOAD32LE(key + 4);
                r[2] = detail::LOAD32LE(key + 8);
                r[3] = detail::LOAD32LE(key + 12);
                s[0] = detail::LOAD32LE(key + 16);
                s[1] = detail::LOAD32LE(key + 20);
                s[2] = detail::LOAD32LE(key + 24);
                s[3] = detail::LOAD32LE(key + 28);

                //Certain bits of r are required to be 0
                r[0] &= 0x0FFFFFFF;
                r[1] &= 0x0FFFFFFC;
                r[2] &= 0x0FFFFFFC;
                r[3] &= 0x0FFFFFFC;

                //The accumulator is set to zero
                a[0] = 0;
                a[1] = 0;
                a[2] = 0;
                a[3] = 0;
                a[4] = 0;
                a[5] = 0;
                a[6] = 0;
                a[7] = 0;

                //Number of bytes in the buffer
                size = 0;
            }

            void update(const void *data, size_t length) {
                size_t n;

                //Process the incoming data
                while(length > 0) {
                   //The buffer can hold at most 16 bytes
                   n = std::min(length, 16 - size);

                   //Copy the data to the buffer
                   std::memcpy(buffer + size, data, n);

                   //Update the Poly1305 context
                   size += n;
                   //Advance the data pointer
                   data = (uint8_t *) data + n;
                   //Remaining bytes to process
                   length -= n;

                   //Process message in 16-byte blocks
                   if(size == 16) {
                      //Transform the 16-byte block
                      process_block();
                      //Empty the buffer
                      size = 0;
                   }
                }
            }

            void final(uint8_t *tag) {
                uint32_t mask;
                uint32_t b[4];

                //Process the last block
                if(size != 0)
                   process_block();

                //Save the accumulator
                b[0] = a[0] & 0xFFFFFFFF;
                b[1] = a[1] & 0xFFFFFFFF;
                b[2] = a[2] & 0xFFFFFFFF;
                b[3] = a[3] & 0xFFFFFFFF;

                //Compute a + 5
                a[0] += 5;

                //Propagate the carry
                a[1] += a[0] >> 32;
                a[2] += a[1] >> 32;
                a[3] += a[2] >> 32;
                a[4] += a[3] >> 32;

                //If (a + 5) >= 2^130, form a mask with the value 0x00000000. Else,
                //form a mask with the value 0xffffffff
                mask = ((a[4] & 0x04) >> 2) - 1;

                //Select between ((a - (2^130 - 5)) % 2^128) and (a % 2^128)
                a[0] = (a[0] & ~mask) | (b[0] & mask);
                a[1] = (a[1] & ~mask) | (b[1] & mask);
                a[2] = (a[2] & ~mask) | (b[2] & mask);
                a[3] = (a[3] & ~mask) | (b[3] & mask);

                //Finally, the value of the secret key s is added to the accumulator
                a[0] += s[0];
                a[1] += s[1];
                a[2] += s[2];
                a[3] += s[3];

                //Propagate the carry
                a[1] += a[0] >> 32;
                a[2] += a[1] >> 32;
                a[3] += a[2] >> 32;
                a[4] += a[3] >> 32;

                //We only consider the least significant bits
                b[0] = a[0] & 0xFFFFFFFF;
                b[1] = a[1] & 0xFFFFFFFF;
                b[2] = a[2] & 0xFFFFFFFF;
                b[3] = a[3] & 0xFFFFFFFF;

                //The result is serialized as a little-endian number, producing
                //the 16 byte tag
                detail::STORE32LE(b[0], tag);
                detail::STORE32LE(b[1], tag + 4);
                detail::STORE32LE(b[2], tag + 8);
                detail::STORE32LE(b[3], tag + 12);

                //Clear the accumulator
                a[0] = 0;
                a[1] = 0;
                a[2] = 0;
                a[3] = 0;
                a[4] = 0;
                a[5] = 0;
                a[6] = 0;
                a[7] = 0;

                //Clear r and s
                r[0] = 0;
                r[1] = 0;
                r[2] = 0;
                r[3] = 0;
                s[0] = 0;
                s[1] = 0;
                s[2] = 0;
                s[3] = 0;
            }

            void process_block() {
                uint32_t a[5];
                uint32_t r[4];
                unsigned int n;

                //Retrieve the length of the last block
                n = this->size;

                //Add one bit beyond the number of octets. For a 16-byte block,
                //this is equivalent to adding 2^128 to the number. For the shorter
                //block, it can be 2^120, 2^112, or any power of two that is evenly
                //divisible by 8, all the way down to 2^8
                this->buffer[n++] = 0x01;

                //If the resulting block is not 17 bytes long (the last block),
                //pad it with zeros
                while(n < 17)
                {
                   this->buffer[n++] = 0x00;
                }

                //Read the block as a little-endian number
                a[0] = detail::LOAD32LE(this->buffer);
                a[1] = detail::LOAD32LE(this->buffer + 4);
                a[2] = detail::LOAD32LE(this->buffer + 8);
                a[3] = detail::LOAD32LE(this->buffer + 12);
                a[4] = this->buffer[16];

                //Add this number to the accumulator
                this->a[0] += a[0];
                this->a[1] += a[1];
                this->a[2] += a[2];
                this->a[3] += a[3];
                this->a[4] += a[4];

                //Propagate the carry
                this->a[1] += this->a[0] >> 32;
                this->a[2] += this->a[1] >> 32;
                this->a[3] += this->a[2] >> 32;
                this->a[4] += this->a[3] >> 32;

                //We only consider the least significant bits
                a[0] = this->a[0] & 0xFFFFFFFF;
                a[1] = this->a[1] & 0xFFFFFFFF;
                a[2] = this->a[2] & 0xFFFFFFFF;
                a[3] = this->a[3] & 0xFFFFFFFF;
                a[4] = this->a[4] & 0xFFFFFFFF;

                //Copy r
                r[0] = this->r[0];
                r[1] = this->r[1];
                r[2] = this->r[2];
                r[3] = this->r[3];

                //Multiply the accumulator by r
                this->a[0] = (uint64_t) a[0] * r[0];
                this->a[1] = (uint64_t) a[0] * r[1] + (uint64_t) a[1] * r[0];
                this->a[2] = (uint64_t) a[0] * r[2] + (uint64_t) a[1] * r[1] + (uint64_t) a[2] * r[0];
                this->a[3] = (uint64_t) a[0] * r[3] + (uint64_t) a[1] * r[2] + (uint64_t) a[2] * r[1] + (uint64_t) a[3] * r[0];
                this->a[4] = (uint64_t) a[1] * r[3] + (uint64_t) a[2] * r[2] + (uint64_t) a[3] * r[1] + (uint64_t) a[4] * r[0];
                this->a[5] = (uint64_t) a[2] * r[3] + (uint64_t) a[3] * r[2] + (uint64_t) a[4] * r[1];
                this->a[6] = (uint64_t) a[3] * r[3] + (uint64_t) a[4] * r[2];
                this->a[7] = (uint64_t) a[4] * r[3];

                //Propagate the carry
                this->a[1] += this->a[0] >> 32;
                this->a[2] += this->a[1] >> 32;
                this->a[3] += this->a[2] >> 32;
                this->a[4] += this->a[3] >> 32;
                this->a[5] += this->a[4] >> 32;
                this->a[6] += this->a[5] >> 32;
                this->a[7] += this->a[6] >> 32;

                //Save the high part of the accumulator
                a[0] = this->a[4] & 0xFFFFFFFC;
                a[1] = this->a[5] & 0xFFFFFFFF;
                a[2] = this->a[6] & 0xFFFFFFFF;
                a[3] = this->a[7] & 0xFFFFFFFF;

                //We only consider the least significant bits
                this->a[0] &= 0xFFFFFFFF;
                this->a[1] &= 0xFFFFFFFF;
                this->a[2] &= 0xFFFFFFFF;
                this->a[3] &= 0xFFFFFFFF;
                this->a[4] &= 0x00000003;

                //Perform fast modular reduction (first pass)
                this->a[0] += a[0];
                this->a[0] += (a[0] >> 2) | (a[1] << 30);
                this->a[1] += a[1];
                this->a[1] += (a[1] >> 2) | (a[2] << 30);
                this->a[2] += a[2];
                this->a[2] += (a[2] >> 2) | (a[3] << 30);
                this->a[3] += a[3];
                this->a[3] += (a[3] >> 2);

                //Propagate the carry
                this->a[1] += this->a[0] >> 32;
                this->a[2] += this->a[1] >> 32;
                this->a[3] += this->a[2] >> 32;
                this->a[4] += this->a[3] >> 32;

                //Save the high part of the accumulator
                a[0] = this->a[4] & 0xFFFFFFFC;

                //We only consider the least significant bits
                this->a[0] &= 0xFFFFFFFF;
                this->a[1] &= 0xFFFFFFFF;
                this->a[2] &= 0xFFFFFFFF;
                this->a[3] &= 0xFFFFFFFF;
                this->a[4] &= 0x00000003;

                //Perform fast modular reduction (second pass)
                this->a[0] += a[0];
                this->a[0] += a[0] >> 2;

                //Propagate the carry
                this->a[1] += this->a[0] >> 32;
                this->a[2] += this->a[1] >> 32;
                this->a[3] += this->a[2] >> 32;
                this->a[4] += this->a[3] >> 32;

                //We only consider the least significant bits
                this->a[0] &= 0xFFFFFFFF;
                this->a[1] &= 0xFFFFFFFF;
                this->a[2] &= 0xFFFFFFFF;
                this->a[3] &= 0xFFFFFFFF;
                this->a[4] &= 0x00000003;
            }

        private:
            uint32_t r[4];
            uint32_t s[4];
            uint64_t a[8];
            uint8_t buffer[17];
            size_t size;
        };

    }

}
