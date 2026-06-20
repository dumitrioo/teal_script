#pragma once

#include "../commondefs.hpp"
#include "../bit_util.hpp"

namespace teal {

    namespace crypt {

        class chacha {
        public:
            chacha() = default;

            chacha(std::uint32_t nr, const std::uint8_t *key, std::size_t keyLen, const std::uint8_t *nonce, std::size_t nonceLen) {
                init(nr, key, keyLen, nonce, nonceLen);
            }

            chacha(chacha const &) = default;
            chacha &operator=(chacha const &) = default;
            chacha(chacha &&) = default;
            chacha &operator=(chacha &&) = default;
            ~chacha() = default;

            /**
             * @brief Initialize ChaCha context using the supplied key and nonce
             * @param[in] nr Number of rounds to be applied (8, 12 or 20)
             * @param[in] key Pointer to the key
             * @param[in] keyLen Length of the key, in bytes (16 or 32)
             * @param[in] nonce Pointer to the nonce
             * @param[in] nonceLen Length of the nonce, in bytes (8 or 12)
             * @return Error code
             **/
            bool init(std::uint32_t rounds, const std::uint8_t *key, std::size_t keyLen, const std::uint8_t *nonce, std::size_t nonceLen) {
                uint32_t *w;

                //Check parameters
                if(key == nullptr || nonce == nullptr)
                   return false;

                //The number of rounds must be 8, 12 or 20
                if(rounds != 8 && rounds != 12 && rounds != 20)
                   return false;

                //Save the number of rounds to be applied
                nr = rounds;

                //Point to the state
                w = state;

                //Check the length of the key
                if(keyLen == 16) {
                   //The first four input words are constants
                   w[0] = 0x61707865;
                   w[1] = 0x3120646E;
                   w[2] = 0x79622D36;
                   w[3] = 0x6B206574;

                   //Input words 4 through 7 are taken from the 128-bit key, by reading
                   //the bytes in little-endian order, in 4-byte chunks
                   w[4] = LOAD32LE(key);
                   w[5] = LOAD32LE(key + 4);
                   w[6] = LOAD32LE(key + 8);
                   w[7] = LOAD32LE(key + 12);

                   //Input words 8 through 11 are taken from the 128-bit key, again by
                   //reading the bytes in little-endian order, in 4-byte chunks
                   w[8] = LOAD32LE(key);
                   w[9] = LOAD32LE(key + 4);
                   w[10] = LOAD32LE(key + 8);
                   w[11] = LOAD32LE(key + 12);
                }
                else if(keyLen == 32)
                {
                   //The first four input words are constants
                   w[0] = 0x61707865;
                   w[1] = 0x3320646E;
                   w[2] = 0x79622D32;
                   w[3] = 0x6B206574;

                   //Input words 4 through 11 are taken from the 256-bit key, by reading
                   //the bytes in little-endian order, in 4-byte chunks
                   w[4] = LOAD32LE(key);
                   w[5] = LOAD32LE(key + 4);
                   w[6] = LOAD32LE(key + 8);
                   w[7] = LOAD32LE(key + 12);
                   w[8] = LOAD32LE(key + 16);
                   w[9] = LOAD32LE(key + 20);
                   w[10] = LOAD32LE(key + 24);
                   w[11] = LOAD32LE(key + 28);
                }
                else
                {
                   //Invalid key length
                   return false;
                }

                //Check the length of the nonce
                if(nonceLen == 8)
                {
                   //Input words 12 and 13 are a block counter, with word 12
                   //overflowing into word 13
                   w[12] = 0;
                   w[13] = 0;

                   //Input words 14 and 15 are taken from an 64-bit nonce, by reading
                   //the bytes in little-endian order, in 4-byte chunks
                   w[14] = LOAD32LE(nonce);
                   w[15] = LOAD32LE(nonce + 4);
                }
                else if(nonceLen == 12)
                {
                   //Input word 12 is a block counter
                   w[12] = 0;

                   //Input words 13 to 15 are taken from an 96-bit nonce, by reading
                   //the bytes in little-endian order, in 4-byte chunks
                   w[13] = LOAD32LE(nonce);
                   w[14] = LOAD32LE(nonce + 4);
                   w[15] = LOAD32LE(nonce + 8);
                }
                else
                {
                   //Invalid nonce length
                   return false;
                }

                //The keystream block is empty
                pos = 0;

                //No error to report
                return true;
            }

            std::vector<std::uint8_t> cipher(void const *input, size_t length) {
                std::vector<std::uint8_t> output{};
                if(input && length) {
                    output.resize(length);
                    cipher(input, &output[0], length);
                }
                return output;
            }

            void cipher(void const *input_ptr, void *output_ptr, size_t length) {
                std::uint8_t const *input{(std::uint8_t const *)input_ptr};
                uint8_t *output{(uint8_t *)output_ptr};
                uint32_t i;
                uint32_t n;
                uint8_t *k;

                //Encryption loop
                while(length > 0) {
                   //Check whether a new keystream block must be generated
                   if(pos == 0 || pos >= 64) {
                      //ChaCha successively calls the ChaCha block function, with the same key
                      //and nonce, and with successively increasing block counter parameters
                      process_block();

                      //Increment block counter
                      state[12]++;

                      //Propagate the carry if necessary
                      if(state[12] == 0) {
                         state[13]++;
                      }

                      //Rewind to the beginning of the keystream block
                      pos = 0;
                   }

                   //Compute the number of bytes to encrypt/decrypt at a time
                   n = std::min(length, 64 - pos);

                   //Valid output pointer?
                   if(output != nullptr) {
                      //Point to the keystream
                      k = (uint8_t *) block + pos;

                      //Valid input pointer?
                      if(input != nullptr) {
                         //XOR the input data with the keystream
                         for(i = 0; i < n; i++) {
                            output[i] = input[i] ^ k[i];
                         }

                         //Advance input pointer
                         input += n;
                      } else {
                         //Output the keystream
                         for(i = 0; i < n; i++) {
                            output[i] = k[i];
                         }
                      }

                      //Advance output pointer
                      output += n;
                   }

                   //Current position in the keystream block
                   pos += n;
                   //Remaining bytes to process
                   length -= n;
                }
            }

        private:
            static std::uint32_t ROL32(std::uint32_t a, int n) {
                return (a << n) | (a >> (32 - n));
            }

            static void CHACHA_QUARTER_ROUND(std::uint32_t &a, std::uint32_t &b, std::uint32_t &c, std::uint32_t &d) {
                a += b;
                d ^= a;
                d = ROL32(d, 16);
                c += d;
                b ^= c;
                b = ROL32(b, 12);
                a += b;
                d ^= a;
                d = ROL32(d, 8);
                c += d;
                b ^= c;
                b = ROL32(b, 7);
            }

            static uint32_t LOAD32LE(std::uint8_t const *p) {
                return bit_util::swap_on_be<uint32_t>{*(uint32_t const *)p}.val;
            }

            void process_block() {
                std::uint32_t i;

                //Copy the state to the working state
                for(i = 0; i < 16; i++) {
                   block[i] = state[i];
                }

                //ChaCha runs 8, 12 or 20 rounds, alternating between column rounds and
                //diagonal rounds
                for(i = 0; i < nr; i += 2) {
                   //The column rounds apply the quarter-round function to the four
                   //columns, from left to right
                   CHACHA_QUARTER_ROUND(block[0], block[4], block[8], block[12]);
                   CHACHA_QUARTER_ROUND(block[1], block[5], block[9], block[13]);
                   CHACHA_QUARTER_ROUND(block[2], block[6], block[10], block[14]);
                   CHACHA_QUARTER_ROUND(block[3], block[7], block[11], block[15]);

                   //The diagonal rounds apply the quarter-round function to the top-left,
                   //bottom-right diagonal, followed by the pattern shifted one place to
                   //the right, for three more quarter-rounds
                   CHACHA_QUARTER_ROUND(block[0], block[5], block[10], block[15]);
                   CHACHA_QUARTER_ROUND(block[1], block[6], block[11], block[12]);
                   CHACHA_QUARTER_ROUND(block[2], block[7], block[8], block[13]);
                   CHACHA_QUARTER_ROUND(block[3], block[4], block[9], block[14]);
                }

                //Add the original input words to the output words
                for(i = 0; i < 16; i++) {
                   block[i] += state[i];
                }

                //Serialize the result by sequencing the words one-by-one in little-endian
                //order
                for(i = 0; i < 16; i++) {
                    block[i] = teal::bit_util::swap_on_be<std::uint32_t>{block[i]}.val;
                }
            }

        private:
            std::uint32_t nr;
            std::uint32_t state[16];
            std::uint32_t block[16];
            std::size_t pos;
        };

    }

}
