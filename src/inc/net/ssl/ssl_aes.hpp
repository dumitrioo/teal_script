#pragma once

#include "../../commondefs.hpp"
#include "../../str_util.hpp"
#include "../../crypto/crypto_utils.hpp"
#include "ssl_str.hpp"
#include "ssl_hash.hpp"
#include "ssl_ptr.hpp"
#include "ssl_rng.hpp"

#include <openssl/evp.h>
#include <openssl/crypto.h>

namespace teal {

    namespace ssl {

        class aes_crydec {
        public:
            static constexpr unsigned int KEY_SIZE = 32;
            static constexpr unsigned int BLOCK_SIZE = 16;

            aes_crydec() {}

            aes_crydec(std::string const &key, std::string const &iv):
                key_{teal::ssl::hash::sha3_256(key)}, iv_{teal::ssl::hash::sha3_128(iv)}
            {}

            aes_crydec(std::string const &key):
                key_{teal::ssl::hash::sha3_256(key)}, iv_{teal::ssl::hash::sha3_128(key)}
            {}

            aes_crydec(std::vector<std::uint8_t> const &key, std::vector<std::uint8_t> const &iv): key_{key}, iv_{iv} {}

            ~aes_crydec() { OPENSSL_cleanse(&key_[0], key_.size()); OPENSSL_cleanse(&iv_[0], iv_.size()); }

            std::vector<std::uint8_t> encrypt(const std::vector<std::uint8_t> &ptext) {
                if(key_.size() != KEY_SIZE) { throw std::runtime_error("aes_crydec: invalid key size"); }
                if(iv_.size() != BLOCK_SIZE) { throw std::runtime_error("aes_crydec: invalid iv size"); }

                if(EVP_EncryptInit_ex(ctx_.get(), EVP_aes_256_cbc(), NULL, key_.data(), iv_.data()) != 1) {
                    throw std::runtime_error("EVP_EncryptInit_ex failed");
                }

                std::vector<std::uint8_t> ctext{};
                ctext.resize(ptext.size() + BLOCK_SIZE);
                int out_len1 = (int)ctext.size();

                if(EVP_EncryptUpdate(ctx_.get(), (std::uint8_t *)&ctext[0], &out_len1, (const std::uint8_t *)&ptext[0], (int)ptext.size()) != 1) {
                    throw std::runtime_error("EVP_EncryptUpdate failed");
                }

                int out_len2 = (int)ctext.size() - out_len1;
                if(EVP_EncryptFinal_ex(ctx_.get(),(std::uint8_t *)&ctext[0]+out_len1, &out_len2) != 1) {
                    throw std::runtime_error("EVP_EncryptFinal_ex failed");
                }

                ctext.resize(out_len1 + out_len2);
                return ctext;
            }

            std::vector<std::uint8_t> decrypt(const std::vector<std::uint8_t> &ctext) {
                if(key_.size() != KEY_SIZE) { throw std::runtime_error("aes_crydec: invalid key size"); }
                if(iv_.size() != BLOCK_SIZE) { throw std::runtime_error("aes_crydec: invalid iv size"); }

                if(EVP_DecryptInit_ex(ctx_.get(), EVP_aes_256_cbc(), NULL, key_.data(), iv_.data()) != 1) {
                    throw std::runtime_error("EVP_DecryptInit_ex failed");
                }

                std::vector<std::uint8_t> rtext{};
                rtext.resize(ctext.size());
                int out_len1 = (int)rtext.size();

                if(EVP_DecryptUpdate(ctx_.get(),(std::uint8_t *)&rtext[0], &out_len1,(const std::uint8_t *)&ctext[0], (int)ctext.size()) != 1) {
                    throw std::runtime_error("EVP_DecryptUpdate failed");
                }

                int out_len2 = (int)rtext.size() - out_len1;
                if(EVP_DecryptFinal_ex(ctx_.get(), (std::uint8_t *)&rtext[0] + out_len1, &out_len2) != 1) {
                    throw std::runtime_error("EVP_DecryptFinal_ex failed");
                }

                rtext.resize(out_len1 + out_len2);
                return rtext;
            }

        private:
            std::vector<std::uint8_t> key_{teal::crypt::gen_rand_bytes(KEY_SIZE)};
            std::vector<std::uint8_t> iv_{teal::crypt::gen_rand_bytes(BLOCK_SIZE)};
            teal::ssl::unique_ptr<EVP_CIPHER_CTX> ctx_{teal::ssl::NEW<EVP_CIPHER_CTX>([]() { return EVP_CIPHER_CTX_new(); })};
        };

    }
}
