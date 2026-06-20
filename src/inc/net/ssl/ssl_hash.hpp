#pragma once

#include "../../commondefs.hpp"
#include "../../str_util.hpp"
#include "ssl_str.hpp"

#include <openssl/sha.h>
#include <openssl/evp.h>

namespace teal {

    namespace ssl {

        namespace hash {

            static std::vector<std::uint8_t> sha256_bytes(void const *bytes, size_t len) {
                unsigned char hash[SHA256_DIGEST_LENGTH];
                SHA256((unsigned char const *)bytes, len, hash);
                return {hash, hash + SHA256_DIGEST_LENGTH};
            }

            static std::vector<std::uint8_t> sha256_cstring(char const *string) {
                return sha256_bytes(string, strlen(string));
            }

            static std::vector<std::uint8_t> sha256(void const *data, size_t len) {
                return sha256_bytes(data, len);
            }

            static std::vector<std::uint8_t> sha256(std::string const &str) {
                return sha256(str.data(), str.size());
            }

            static std::vector<std::uint8_t> sha256(std::vector<std::uint8_t> const &vec) {
                return sha256(vec.data(), vec.size());
            }

            static std::vector<std::uint8_t> sha256(char const *cstr) {
                std::string s{cstr};
                return sha256(s.data(), s.size());
            }



            static std::vector<std::uint8_t> sha512_bytes(void const *bytes, size_t len) {
                unsigned char hash[SHA512_DIGEST_LENGTH];
                SHA512((unsigned char const *)bytes, len, hash);
                return {hash, hash + SHA512_DIGEST_LENGTH};
            }

            static std::vector<std::uint8_t> sha512_cstring(char *string) {
                return sha512_bytes(string, strlen(string));
            }

            static std::vector<std::uint8_t> sha512(void const *data, size_t len) {
                return sha512_bytes(data, len);
            }

            static std::vector<std::uint8_t> sha512(std::string const &str) {
                return sha512(str.data(), str.size());
            }

            static std::vector<std::uint8_t> sha512(std::vector<std::uint8_t> const &vec) {
                return sha512(vec.data(), vec.size());
            }


            static std::vector<std::uint8_t> sha3_512(void const *bytes, size_t len) {
                uint32_t digest_length = SHA512_DIGEST_LENGTH;
                const EVP_MD* algorithm = EVP_sha3_512();
                std::unique_ptr<uint8_t, std::function<void(uint8_t *)>> digest{static_cast<uint8_t *>(OPENSSL_malloc(digest_length)), [](uint8_t *d) { OPENSSL_free(d); } };
                std::unique_ptr<EVP_MD_CTX, std::function<void(EVP_MD_CTX *)>> context{EVP_MD_CTX_new(), [](EVP_MD_CTX *c) { EVP_MD_CTX_destroy(c); }};
                EVP_DigestInit_ex(context.get(), algorithm, nullptr);
                EVP_DigestUpdate(context.get(), bytes, len);
                EVP_DigestFinal_ex(context.get(), digest.get(), &digest_length);
                return {digest.get(), digest.get() + digest_length};
            }

            static std::vector<std::uint8_t> sha3_512(std::string const &s) {
                return sha3_512(s.data(), s.size());
            }

            static std::vector<std::uint8_t> sha3_512(std::vector<std::uint8_t> const &v) {
                return sha3_512(v.data(), v.size());
            }



            static std::vector<std::uint8_t> sha3_256(void const *bytes, size_t len) {
                uint32_t digest_length = SHA256_DIGEST_LENGTH;
                const EVP_MD* algorithm = EVP_sha3_256();
                std::unique_ptr<uint8_t, std::function<void(uint8_t *)>> digest{static_cast<uint8_t *>(OPENSSL_malloc(digest_length)), [](uint8_t *d) { OPENSSL_free(d); } };
                std::unique_ptr<EVP_MD_CTX, std::function<void(EVP_MD_CTX *)>> context{EVP_MD_CTX_new(), [](EVP_MD_CTX *c) { EVP_MD_CTX_destroy(c); }};
                EVP_DigestInit_ex(context.get(), algorithm, nullptr);
                EVP_DigestUpdate(context.get(), bytes, len);
                EVP_DigestFinal_ex(context.get(), digest.get(), &digest_length);
                return {digest.get(), digest.get() + digest_length};
            }

            static std::vector<std::uint8_t> sha3_256(std::string const &s) {
                return sha3_256(s.data(), s.size());
            }

            static std::vector<std::uint8_t> sha3_256(std::vector<std::uint8_t> const &v) {
                return sha3_256(v.data(), v.size());
            }



            static std::vector<std::uint8_t> sha3_128(void const *bytes, size_t len) {
                uint32_t digest_length = SHA256_DIGEST_LENGTH;
                const EVP_MD* algorithm = EVP_sha3_256();
                std::unique_ptr<uint8_t, std::function<void(uint8_t *)>> digest{static_cast<uint8_t *>(OPENSSL_malloc(digest_length)), [](uint8_t *d) { OPENSSL_free(d); } };
                std::unique_ptr<EVP_MD_CTX, std::function<void(EVP_MD_CTX *)>> context{EVP_MD_CTX_new(), [](EVP_MD_CTX *c) { EVP_MD_CTX_destroy(c); }};
                EVP_DigestInit_ex(context.get(), algorithm, nullptr);
                EVP_DigestUpdate(context.get(), bytes, len);
                EVP_DigestFinal_ex(context.get(), digest.get(), &digest_length);
                return {digest.get() + SHA_LBLOCK / 2, digest.get() + SHA_LBLOCK + SHA_LBLOCK / 2};
            }

            static std::vector<std::uint8_t> sha3_128(std::string const &s) {
                return sha3_128(s.data(), s.size());
            }

            static std::vector<std::uint8_t> sha3_128(std::vector<std::uint8_t> const &v) {
                return sha3_128(v.data(), v.size());
            }



            static std::vector<std::uint8_t> hash_at_level(std::vector<std::uint8_t> const &data, int64_t level) {
                std::vector<std::uint8_t> res = teal::ssl::hash::sha3_256(data);
                for(int64_t i = 0; i < level - 1LL; ++i) {
                    res = teal::ssl::hash::sha3_256(res);
                }
                return res;
            }

            static std::vector<std::uint8_t> hash_at_level(std::string const &data, int64_t level) {
                std::vector<std::uint8_t> res = teal::ssl::hash::sha3_256(data);
                for(int64_t i = 0; i < level - 1LL; ++i) {
                    res = teal::ssl::hash::sha3_256(res);
                }
                return res;
            }

            static std::vector<std::uint8_t> hash_at_level(char const *data, int64_t level) {
                std::vector<std::uint8_t> res = teal::ssl::hash::sha3_256(std::string{data});
                for(int64_t i = 0; i < level - 1LL; ++i) {
                    res = teal::ssl::hash::sha3_256(res);
                }
                return res;
            }

        }
    }
}
