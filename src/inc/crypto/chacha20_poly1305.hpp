#pragma once

#include "../commondefs.hpp"
#include "chacha.hpp"
#include "poly1305.hpp"

#define STORE64LE(a, p) \
   ((uint8_t *)(p))[0] = ((uint64_t)(a) >> 0) & 0xFFU, \
   ((uint8_t *)(p))[1] = ((uint64_t)(a) >> 8) & 0xFFU, \
   ((uint8_t *)(p))[2] = ((uint64_t)(a) >> 16) & 0xFFU, \
   ((uint8_t *)(p))[3] = ((uint64_t)(a) >> 24) & 0xFFU, \
   ((uint8_t *)(p))[4] = ((uint64_t)(a) >> 32) & 0xFFU, \
   ((uint8_t *)(p))[5] = ((uint64_t)(a) >> 40) & 0xFFU, \
   ((uint8_t *)(p))[6] = ((uint64_t)(a) >> 48) & 0xFFU, \
   ((uint8_t *)(p))[7] = ((uint64_t)(a) >> 56) & 0xFFU

namespace teal {

    namespace crypt {

        struct chacha20_poly1305 {

            /**
             * @brief Authenticated encryption using ChaCha20Poly1305
             * @param[in] k key
             * @param[in] kLen Length of the key
             * @param[in] n Nonce
             * @param[in] nLen Length of the nonce
             * @param[in] a Additional authenticated data
             * @param[in] aLen Length of the additional data
             * @param[in] p Plaintext to be encrypted
             * @param[out] c Ciphertext resulting from the encryption
             * @param[in] length Total number of data bytes to be encrypted
             * @param[out] t MAC resulting from the encryption process
             * @param[in] tLen Length of the MAC
             * @return Error code
             **/
            static bool encrypt(const uint8_t *k, size_t kLen,
                                const uint8_t *n, size_t nLen, const uint8_t *aad, size_t aLen,
                                const uint8_t *p, uint8_t *c, size_t length, uint8_t *t, size_t tLen) {
                size_t paddingLen{};
                chacha chachaContext{};
                poly1305 poly1305Context{};
                uint8_t temp[32]{};

                //Check the length of the message-authentication code
                if(tLen != 16) {
                   return false;
                }

                //Initialize ChaCha20 context
                //Any error to report?
                if(!chachaContext.init(20, k, kLen, n, nLen))
                   return false;

                //First, a Poly1305 one-time key is generated from the 256-bit key
                //and nonce
                chachaContext.cipher(nullptr, temp, 32);

                //The other 256 bits of the Chacha20 block are discarded
                chachaContext.cipher(nullptr, nullptr, 32);

                //Next, the ChaCha20 encryption function is called to encrypt the
                //plaintext, using the same key and nonce
                chachaContext.cipher(p, c, length);

                //Initialize the Poly1305 function with the key calculated above
                poly1305Context.init(temp);

                //Compute MAC over the AAD
                poly1305Context.update(aad, aLen);

                //If the length of the AAD is not an integral multiple of 16 bytes,
                //then padding is required
                if((aLen % 16) != 0) {
                   //Compute the number of padding bytes
                   paddingLen = 16 - (aLen % 16);

                   //The padding is up to 15 zero bytes, and it brings the total length
                   //so far to an integral multiple of 16
                   std::memset(temp, 0, paddingLen);

                   //Compute MAC over the padding
                   poly1305Context.update(temp, paddingLen);
                }

                //Compute MAC over the ciphertext
                poly1305Context.update(c, length);

                //If the length of the ciphertext is not an integral multiple of 16 bytes,
                //then padding is required
                if((length % 16) != 0) {
                   //Compute the number of padding bytes
                   paddingLen = 16 - (length % 16);

                   //The padding is up to 15 zero bytes, and it brings the total length
                   //so far to an integral multiple of 16
                   std::memset(temp, 0, paddingLen);

                   //Compute MAC over the padding
                   poly1305Context.update(temp, paddingLen);
                }

                //Encode the length of the AAD as a 64-bit little-endian integer
                STORE64LE(aLen, temp);
                //Compute MAC over the length field
                poly1305Context.update(temp, sizeof(uint64_t));

                //Encode the length of the ciphertext as a 64-bit little-endian integer
                STORE64LE(length, temp);
                //Compute MAC over the length field
                poly1305Context.update(temp, sizeof(uint64_t));

                //Compute message-authentication code
                poly1305Context.final(t);

                //Successful encryption
                return true;
            }

            /**
             * @brief Authenticated decryption using ChaCha20Poly1305
             * @param[in] k key
             * @param[in] kLen Length of the key
             * @param[in] n Nonce
             * @param[in] nLen Length of the nonce
             * @param[in] a Additional authenticated data
             * @param[in] aLen Length of the additional data
             * @param[in] c Ciphertext to be decrypted
             * @param[out] p Plaintext resulting from the decryption
             * @param[in] length Total number of data bytes to be decrypted
             * @param[in] t MAC to be verified
             * @param[in] tLen Length of the MAC
             * @return Error code
             **/
            static bool decrypt(const uint8_t *k, size_t kLen,
                         const uint8_t *n, size_t nLen, const uint8_t *aad, size_t aLen,
                         const uint8_t *c, uint8_t *p, size_t length, const uint8_t *t, size_t tLen) {
                uint8_t mask;
                size_t i;
                size_t paddingLen;
                chacha chachaContext;
                poly1305 poly1305Context;
                uint8_t temp[32];

                //Check the length of the message-authentication code
                if(tLen != 16)
                   return false;

                //Initialize ChaCha20 context
                //Any error to report?
                if(!chachaContext.init(20, k, kLen, n, nLen))
                   return false;

                //First, a Poly1305 one-time key is generated from the 256-bit key
                //and nonce
                chachaContext.cipher(nullptr, temp, 32);

                //The other 256 bits of the Chacha20 block are discarded
                chachaContext.cipher(nullptr, nullptr, 32);

                //Initialize the Poly1305 function with the key calculated above
                poly1305Context.init(temp);

                //Compute MAC over the AAD
                poly1305Context.update(aad, aLen);

                //If the length of the AAD is not an integral multiple of 16 bytes,
                //then padding is required
                if((aLen % 16) != 0) {
                   //Compute the number of padding bytes
                   paddingLen = 16 - (aLen % 16);

                   //The padding is up to 15 zero bytes, and it brings the total length
                   //so far to an integral multiple of 16
                   std::memset(temp, 0, paddingLen);

                   //Compute MAC over the padding
                   poly1305Context.update(temp, paddingLen);
                }

                //Compute MAC over the ciphertext
                poly1305Context.update(c, length);

                //If the length of the ciphertext is not an integral multiple of 16 bytes,
                //then padding is required
                if((length % 16) != 0)
                {
                   //Compute the number of padding bytes
                   paddingLen = 16 - (length % 16);

                   //The padding is up to 15 zero bytes, and it brings the total length
                   //so far to an integral multiple of 16
                   std::memset(temp, 0, paddingLen);

                   //Compute MAC over the padding
                   poly1305Context.update(temp, paddingLen);
                }

                //Encode the length of the AAD as a 64-bit little-endian integer
                STORE64LE(aLen, temp);
                //Compute MAC over the length field
                poly1305Context.update(temp, sizeof(uint64_t));

                //Encode the length of the ciphertext as a 64-bit little-endian integer
                STORE64LE(length, temp);
                //Compute MAC over the length field
                poly1305Context.update(temp, sizeof(uint64_t));

                //Compute message-authentication code
                poly1305Context.final(temp);

                //Finally, we decrypt the ciphertext
                chachaContext.cipher(c, p, length);

                //The calculated tag is bitwise compared to the received tag. The
                //message is authenticated if and only if the tags match
                for(mask = 0, i = 0; i < tLen; i++) {
                   mask |= temp[i] ^ t[i];
                }

                //Return status code
                return mask == 0;
            }

        };

    }

}

#undef STORE64LE
