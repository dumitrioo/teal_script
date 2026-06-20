#pragma once

#include "../../commondefs.hpp"

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/ossl_typ.h>
#include <openssl/rsa.h>
#define OPENSSL_NO_EC2M
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/ecdh.h>

#include "../../serialization.hpp"

#include "../../base64.hpp"
#include "../../base16.hpp"

#include "ssl_aes.hpp"
#include "ssl_hash.hpp"
#include "ssl_ptr.hpp"
#include "ssl_rng.hpp"
#include "ssl_str.hpp"

#include "../../file_util.hpp"

#include "../../crypto/crypto_utils.hpp"
#include "../../crypto/chacha20_poly1305.hpp"
#include "../../crypto/sha3_256.hpp"
#include "../../crypto/sha3_512.hpp"

constexpr int TMP_BUFF_SIZE = 4096;

namespace teal {

    namespace ssl {

        static std::vector<std::uint8_t> gen_rand_bytes(std::size_t num_bytes) {
            std::vector<std::uint8_t> res{};
            if(num_bytes) {
                res.resize(num_bytes);
                RAND_bytes(&res[0], num_bytes);
            }
            return res;
        }

        template<typename T>
        class crypto_wrapper {
        public:
            using value_type = T;
            crypto_wrapper() = default;
            crypto_wrapper(T const &v): v_{v} {}
            crypto_wrapper &operator=(T const &v) { v_ = v; }
            crypto_wrapper(T &&v): v_{std::move(v)} {}
            crypto_wrapper &operator=(T &&v) { v_ = std::move(v); }
            ~crypto_wrapper() { for(auto &i: v_) { i = typename T::value_type{}; } }
            template<typename...E> crypto_wrapper(E&&...v): v_{std::forward<E>(v)...} {}

            T *operator->() { return &v_; }
            T &operator*() { return v_; }
            T const &operator*() const { return v_; }
            operator T&() { return v_; }
            operator T const &() const { return v_; }

        private:
            T v_{};
        };

        DEFINE_RUNTIME_ERROR_CLASS(ssl_error)

        static bool init() {
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_ssl_algorithms();
            ERR_load_BIO_strings();
            OpenSSL_add_all_algorithms();
            ERR_load_crypto_strings();
            return true;
        }

        static std::string error_string(unsigned long e) {
            std::vector<char> buff(8192);
            ERR_error_string_n(e, &buff[0], buff.size());
            return buff.data();
        }

        static std::string last_error_string() {
            return error_string(ERR_get_error());
        }

        static void throw_last_error() {
            char err_msg_buf[TMP_BUFF_SIZE];
            auto err_code{ERR_get_error()};
            ERR_error_string_n(err_code, err_msg_buf, TMP_BUFF_SIZE);
            throw ssl_error(err_msg_buf);
        }

        static void throw_last_error_if_nonzero() {
            char err_msg_buf[TMP_BUFF_SIZE];
            auto err_code{ERR_get_error()};
            if(err_code) {
                ERR_error_string_n(err_code, err_msg_buf, TMP_BUFF_SIZE);
                throw ssl_error(err_msg_buf);
            }
        }

        static void throw_ssl_error() {
            char err_msg_buf[TMP_BUFF_SIZE];
            auto err_code{ERR_get_error()};
            ERR_error_string_n(err_code, err_msg_buf, TMP_BUFF_SIZE);
            throw ssl_error(err_msg_buf);
        }

        static std::pair<std::string, std::string> generate_rsa_key_pair(int bits = 4096) {
            try {
                bool res{false};
                unsigned long e{RSA_F4};
                ssl_ptr<BIGNUM> bne{teal::ssl::NEW<BIGNUM>([&]() { return BN_new(); })};
                res = BN_set_word(bne.get(), e) == 1;
                if(!res) { throw_last_error(); }
                ssl_ptr<RSA> r{teal::ssl::NEW<RSA>([&]() { return RSA_new(); })};
                res = RSA_generate_key_ex(r.get(), bits, bne.get(), nullptr) == 1;
                if(!res) { throw_last_error(); }
                ssl_ptr<BIO> bp_public{teal::ssl::NEW<BIO>([&]() { return BIO_new(BIO_s_mem()); })};
                res = PEM_write_bio_RSAPublicKey(bp_public.get(), r.get()) == 1;
                if(!res) { throw_last_error(); }
                ssl_ptr<BIO> bp_private{teal::ssl::NEW<BIO>([&]() { return BIO_new(BIO_s_mem()); })};
                res = PEM_write_bio_RSAPrivateKey(bp_private.get(), r.get(), nullptr, nullptr, 0, nullptr, nullptr) == 1;
                if(!res) { throw_last_error(); }
                std::vector<char> priv{};
                std::vector<char> publ{};
                {
                    BUF_MEM *bptr;
                    BIO_get_mem_ptr(bp_public.get(), &bptr);
                    publ.resize(bptr->length);
                    BIO_read(bp_public.get(), &publ[0], bptr->length);
                }
                {
                    BUF_MEM *bptr;
                    BIO_get_mem_ptr(bp_private.get(), &bptr);
                    priv.resize(bptr->length);
                    BIO_read(bp_private.get(), &priv[0], bptr->length);
                }
                return {std::string{priv.begin(), priv.end()}, std::string{publ.begin(), publ.end()}};
            } catch(...) {
            }
            return {};
        }

        static std::vector<std::uint8_t> rsa_encrypt(std::vector<std::uint8_t> const &plain_data, std::string const &pubkey) {
            unsigned char const *rsaPublicKeyChar = reinterpret_cast<unsigned char const *>(pubkey.data());
            ssl_ptr<BIO> rsaPublicBIO{teal::ssl::NEW<BIO>([&]() { return BIO_new_mem_buf(rsaPublicKeyChar, -1); })};
            RSA *rsaPublicKey = nullptr;
            ssl_ptr<RSA> rsa_pub_read{teal::ssl::NEW<RSA>([&]() { return PEM_read_bio_RSAPublicKey(rsaPublicBIO.get(), &rsaPublicKey, 0, nullptr); })};
            if(rsa_pub_read) {
                int encrypt_len{-1};
                std::vector<std::uint8_t> encrypted{}; encrypted.resize(1024 * 128);
                if((encrypt_len = RSA_public_encrypt(plain_data.size(), (unsigned char const *)plain_data.data(), (unsigned char*)&encrypted[0], rsa_pub_read.get(), RSA_PKCS1_PADDING)) == -1) {
                    throw_last_error();
                }
                encrypted.resize(encrypt_len);
                return encrypted;
            }
            return {};
        }

        static std::vector<std::uint8_t> rsa_decrypt(std::vector<std::uint8_t> const &enc_data, std::string const &privkey) {
            unsigned char const *rsaPrivateKeyChar = reinterpret_cast<unsigned char const *>(privkey.data());
            ssl_ptr<BIO> rsaPrivateBIO{teal::ssl::NEW<BIO>([&]() { return BIO_new_mem_buf(rsaPrivateKeyChar, -1); })};
            RSA *rsaPrivateKey = nullptr;
            unique_ptr<RSA> rsa_priv_read{teal::ssl::NEW<RSA>([&]() { return PEM_read_bio_RSAPrivateKey(rsaPrivateBIO.get(), &rsaPrivateKey, nullptr, nullptr); })};
            if(rsa_priv_read) {
                std::vector<std::uint8_t> decrypted{}; decrypted.resize(1024 * 128);
                int decrypt_len{-1};
                if((decrypt_len = RSA_private_decrypt(enc_data.size(), (unsigned char const *)enc_data.data(), (unsigned char *)&decrypted[0], rsa_priv_read.get(), RSA_PKCS1_PADDING)) == -1) {
                    throw_last_error();
                }
                decrypted.resize(decrypt_len);
                return decrypted;
            }
            return {};
        }

        static ssl_ptr<RSA> priv_pem_to_rsa_key(const std::string& pemkey) {
            RSA *rsa_key = nullptr;
            if(pemkey.empty()) {
                throw ssl_error{"invalid key string"};
            }
            ssl_ptr<BIO> bio{BIO_new(BIO_s_mem())};
            int bio_write_ret = BIO_write(bio.get(), static_cast<const char*>(pemkey.c_str()), pemkey.size());
            if (bio_write_ret <= 0) {
                throw_last_error();
            }
            if(!PEM_read_bio_RSAPrivateKey(bio.get(), &rsa_key, nullptr, nullptr)) {
                throw_last_error();
            }
            return ssl_ptr<RSA>{rsa_key};
        }

        static ssl_ptr<RSA> pub_pem_to_rsa_key(const std::string& pemkey) {
            RSA *rsa_key = nullptr;
            if(pemkey.empty()) {
                throw ssl_error{"invalid key string"};
            }
            ssl_ptr<BIO> bio{BIO_new(BIO_s_mem())};
            int bio_write_ret = BIO_write(bio.get(), static_cast<const char*>(pemkey.c_str()), pemkey.size());
            if (bio_write_ret <= 0) {
                throw_last_error();
            }
            if(!PEM_read_bio_RSAPublicKey(bio.get(), &rsa_key, nullptr, nullptr)) {
                throw_last_error();
            }
            return ssl_ptr<RSA>{rsa_key};
        }

        static std::string rsa_priv_key_to_pem(RSA *r) {
            try {
                bool res{false};
                ssl_ptr<BIO> bp_private{teal::ssl::NEW<BIO>([&]() { return BIO_new(BIO_s_mem()); })};
                res = PEM_write_bio_RSAPrivateKey(bp_private.get(), r, nullptr, nullptr, 0, nullptr, nullptr) == 1;
                if(!res) { throw_last_error(); }
                std::vector<char> priv{};
                {
                    BUF_MEM *bptr;
                    BIO_get_mem_ptr(bp_private.get(), &bptr);
                    priv.resize(bptr->length);
                    BIO_read(bp_private.get(), &priv[0], bptr->length);
                }
                return std::string{priv.begin(), priv.end()};
            } catch(...) {
            }
            return {};
        }

        static std::string rsa_pub_key_to_pem(RSA *r) {
            try {
                bool res{false};
                ssl_ptr<BIO> bp_public{teal::ssl::NEW<BIO>([&]() { return BIO_new(BIO_s_mem()); })};
                res = PEM_write_bio_RSAPublicKey(bp_public.get(), r) == 1;
                if(!res) { throw_last_error(); }
                std::vector<char> publ{};
                {
                    BUF_MEM *bptr;
                    BIO_get_mem_ptr(bp_public.get(), &bptr);
                    publ.resize(bptr->length);
                    BIO_read(bp_public.get(), &publ[0], bptr->length);
                }
                return std::string{publ.begin(), publ.end()};
            } catch(...) {
            }
            return {};
        }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //            ecc                                                                                                  //
        static ssl_ptr<EC_KEY> priv_pem_to_ec_key(const std::string& pemkey) {
            EC_KEY *ec_key = nullptr;
            if(pemkey.empty()) {
                throw ssl_error{"invalid key string"};
            }
            ssl_ptr<BIO> bio{BIO_new(BIO_s_mem())};
            int bio_write_ret = BIO_write(bio.get(), static_cast<const char*>(pemkey.c_str()), pemkey.size());
            if (bio_write_ret <= 0) {
                throw_last_error();
            }
            if(!PEM_read_bio_ECPrivateKey(bio.get(), &ec_key, nullptr, nullptr)) {
                throw_last_error();
            }
            EC_KEY_set_asn1_flag(ec_key, OPENSSL_EC_NAMED_CURVE);
            return ssl_ptr<EC_KEY>{ec_key};
        }

        static ssl_ptr<EC_KEY> pub_pem_to_ec_key(const std::string& pemkey) {
            EC_KEY *ec_key = nullptr;
            if(pemkey.empty()) {
                throw ssl_error{"invalid key string"};
            }
            ssl_ptr<BIO> bio{BIO_new(BIO_s_mem())};
            int bio_write_ret = BIO_write(bio.get(), static_cast<const char*>(pemkey.c_str()), pemkey.size());
            if (bio_write_ret <= 0) {
                throw_last_error();
            }
            if(!PEM_read_bio_EC_PUBKEY(bio.get(), &ec_key, nullptr, nullptr)) {
                throw_last_error();
            }
            EC_KEY_set_asn1_flag(ec_key, OPENSSL_EC_NAMED_CURVE);
            return ssl_ptr<EC_KEY>{ec_key};
        }

        static std::string ec_key_to_pem(EC_KEY *ec_key) {
            ssl_ptr<BIO> outbio{BIO_new(BIO_s_mem())};

            bool is_private{false};
            if (EC_KEY_get0_private_key(ec_key)) {
                is_private = true;
            }

            if(is_private) {
                PEM_write_bio_ECPrivateKey(outbio.get(), ec_key, nullptr, nullptr, 0, nullptr, nullptr);
            } else {
                PEM_write_bio_EC_PUBKEY(outbio.get(), ec_key);
            }

            std::string keyStr;
            int priKeyLen = BIO_pending(outbio.get());
            keyStr.resize(priKeyLen);
            BIO_read(outbio.get(), (void*)&(keyStr.front()), priKeyLen);
            return keyStr;
        }

        static bool is_valid_pub_key(const std::string &pemkey) {
            try {
                if(!pemkey.empty()) {
                    ssl_ptr<EC_KEY> eck{pub_pem_to_ec_key(pemkey)};
                    if(eck) {
                        return EC_KEY_check_key(eck.get()) != 0;
                    }
                }
            } catch(...) {
            }
            return false;
        }

        static bool is_valid_priv_key(const std::string& pemkey) {
            try {
                if(!pemkey.empty()) {
                    ssl_ptr<EC_KEY> eck{priv_pem_to_ec_key(pemkey)};
                    if(eck) {
                        return EC_KEY_check_key(eck.get()) != 0;
                    }
                }
            } catch(...) {
            }
            return false;
        }

        static std::pair<std::string, std::string> generate_ecc_key_pair(std::string const &curve = std::string{"secp521r1"}) {
            int eccgrp = OBJ_txt2nid(curve.c_str());
            ssl_ptr<EC_KEY> ecc_priv{EC_KEY_new_by_curve_name(eccgrp)};
            if(!ecc_priv) {
                throw_last_error();
            }

            EC_KEY_set_asn1_flag(ecc_priv.get(), OPENSSL_EC_NAMED_CURVE);

            if(!(EC_KEY_generate_key(ecc_priv.get()))) {
                throw_last_error();
            }

            ssl_ptr<EC_KEY> ecc_pub{EC_KEY_new()};

            EC_GROUP const *grp{EC_KEY_get0_group(ecc_priv.get())};
            if(!EC_KEY_set_group(ecc_pub.get(), grp)) {
                throw_last_error();
            }

            const EC_POINT *pt{EC_KEY_get0_public_key(ecc_priv.get())};
            if(!EC_KEY_set_public_key(ecc_pub.get(), pt)) {
                throw_last_error();
            }

            return {ec_key_to_pem(ecc_priv.get()), ec_key_to_pem(ecc_pub.get())};
        }

        static std::vector<std::uint8_t> EC_MV_encrypt(std::vector<std::uint8_t> const &plaintext, EC_KEY *pubkey) {
            const EC_GROUP *curve;
            const EC_POINT *q;
            int bits, i = 0, err;
            unsigned char buffer[250];

            curve = EC_KEY_get0_group(pubkey);
            if(curve) {
                bits = EC_GROUP_get_degree(curve);
            } else {
                throw std::runtime_error{"no curve in key"};
            }
            if(plaintext.empty()) { throw std::runtime_error{"wrong plaintext - zero size"}; }
            if(plaintext.size() % 2) { throw std::runtime_error{"wrong plaintext - has odd length"}; }
            if((int)plaintext.size() > 2 * bits / 8) { throw std::runtime_error{"plaintext too long for this key"}; }

            unique_ptr<BIGNUM> p{BN_new()}, a{BN_new()}, b{BN_new()}, ks{BN_new()}, o1{BN_new()}, z1{BN_new()}, z2{BN_new()}, y1{BN_new()}, y2{BN_new()}, x1{BN_new()}, x2{BN_new()}, ord{BN_new()};
            unique_ptr<BN_CTX> ctx{BN_CTX_new()};

            unique_ptr<EC_POINT> y0{EC_POINT_new(curve)};
            unique_ptr<EC_POINT> z{EC_POINT_new(curve)};

            BN_bin2bn(plaintext.data(), plaintext.size() / 2, x1.get());
            BN_bin2bn(&plaintext[plaintext.size() / 2], plaintext.size() / 2, x2.get());

            q = EC_KEY_get0_public_key(pubkey);
            EC_GROUP_get_order(curve, ord.get(), ctx.get());

            EC_GROUP_get_curve_GFp(curve, p.get(), a.get(), b.get(), ctx.get());

            BN_sub(o1.get(), ord.get(), BN_value_one());

            do {
                if(i >= 10) {
                    break;
                }
                RAND_bytes(buffer, bits / 8);
                BN_bin2bn(buffer, bits / 8, ks.get());
                ++i;
            } while(BN_cmp(BN_value_one(), ks.get()) >= 0 || BN_cmp(o1.get(), ks.get()) <= 0);

            if(i>=10) {
                throw std::runtime_error{"ks is wrong, error in do-while"};
            }

            err = EC_POINT_mul(curve, y0.get(), ks.get(), nullptr, nullptr, ctx.get());
            if(err == 0) {
                throw_last_error();
            }

            err = EC_POINT_mul(curve, z.get(), nullptr, q, ks.get(), ctx.get());
            if(err == 0) {
                throw_last_error();
            }

            err = EC_POINT_get_affine_coordinates_GFp(curve, z.get(), z1.get(), z2.get(), ctx.get());
            if(err == 0) {
                throw_last_error();
            }

            BN_mod_mul(y1.get(), z1.get(), x1.get(), p.get(), ctx.get());
            BN_mod_mul(y2.get(), z2.get(), x2.get(), p.get(), ctx.get());

            // std::vector<std::uint8_t> cipher{};
            teal::serializer cipher_serial{};

            char *hpt{EC_POINT_point2hex(curve, y0.get(), POINT_CONVERSION_UNCOMPRESSED, ctx.get())};
            try { cipher_serial << hex_str_to_data(hpt); } catch(...) {}
            OPENSSL_free(hpt);
            cipher_serial << (std::uint8_t)(plaintext.size() / 2);
            {
                std::vector<std::uint8_t> bnb{}; bnb.resize(1024);
                int cnt{BN_bn2bin(y1.get(), &bnb[0])};
                bnb.resize(cnt);
                cipher_serial << bnb;
            }
            {
                std::vector<std::uint8_t> bnb{}; bnb.resize(1024);
                int cnt{BN_bn2bin(y2.get(), &bnb[0])};
                bnb.resize(cnt);
                cipher_serial << bnb;
            }
            // cipher = cipher_serial.data_vec();
            // return cipher;
            return std::move(cipher_serial).data_vec();
        }

        static std::vector<std::uint8_t> EC_MV_encrypt(std::vector<std::uint8_t> const &plain_data, std::string const &pubkey) {
            try {
                ssl_ptr<EC_KEY> ec_pub{pub_pem_to_ec_key(pubkey)};
                return EC_MV_encrypt(plain_data, ec_pub.get());
            } catch(...) {
            }
            return {};
        }

        static std::vector<std::uint8_t> prepend_zeroes(std::vector<std::uint8_t> const &v, std::size_t const new_size) {
            std::vector<std::uint8_t> res{v};
            if(res.size() < new_size) {
                teal::bit_util::inplace_swap(&res[0], res.size());
                while(res.size() < new_size) {
                    res.push_back(0);
                }
                teal::bit_util::inplace_swap(&res[0], res.size());
            }
            return res;
        }

        static std::vector<std::uint8_t> EC_MV_decrypt(std::vector<std::uint8_t> const &cipher, EC_KEY *privkey) {
            const EC_GROUP *curve{EC_KEY_get0_group(privkey)};
            if(!curve) {
                throw std::runtime_error{"no curve in key"};
            }
            if(cipher.size() == 0) {
                throw std::runtime_error{"invalid cipher"};
            }
            // int bits{EC_GROUP_get_degree(curve)};

            unique_ptr<BIGNUM> p{BN_new()}, a{BN_new()}, b{BN_new()}, z1{BN_new()}, z2{BN_new()}, x1{BN_new()}, x2{BN_new()};
            unique_ptr<BN_CTX> ctx{BN_CTX_new()};

            unique_ptr<EC_POINT> y0{EC_POINT_new(curve)}, z{EC_POINT_new(curve)};

            BIGNUM const *d{EC_KEY_get0_private_key(privkey)};
            EC_GROUP_get_curve_GFp(curve, p.get(), a.get(), b.get(), ctx.get());

            BIGNUM *y1{BN_new()};
            unique_ptr<BIGNUM> y1_deleter{y1};
            BIGNUM *y2{BN_new()};
            unique_ptr<BIGNUM> y2_deleter{y2};
            int digsiz{0};
            {
                teal::serial_reader cipher_serial{&cipher[0], cipher.size()};
                teal::serial_reader::const_iterator it{cipher_serial.begin()};
                {
                    std::string pts{data_to_hex_str(it->as_bytevec())};
                    EC_POINT *pt{EC_POINT_hex2point(curve, pts.c_str(), y0.get(), ctx.get())};
                    if(!pt) { throw_last_error(); }
                }

                ++it;
                digsiz = it->as_number();

                {
                    ++it;
                    std::vector<std::uint8_t> v{it->as_bytevec()};
                    BN_bin2bn((const unsigned char *)v.data(), v.size(), y1);
                }

                {
                    ++it;
                    std::vector<std::uint8_t> v{it->as_bytevec()};
                    BN_bin2bn((const unsigned char *)v.data(), v.size(), y2);
                }

            }

            if(!EC_POINT_is_on_curve(curve, y0.get(), ctx.get())) { throw_last_error(); }

            int err{EC_POINT_mul(curve, z.get(), nullptr, y0.get(), d, ctx.get())};
            if(err == 0) { throw_last_error(); }

            err = EC_POINT_get_affine_coordinates_GFp(curve, z.get(), z1.get(), z2.get(), ctx.get());
            if(err == 0) { throw_last_error(); }

            BN_mod_inverse(a.get(), z1.get(), p.get(), ctx.get());
            BN_mod_inverse(b.get(), z2.get(), p.get(), ctx.get());

            BN_mod_mul(x1.get(), a.get(), y1, p.get(), ctx.get());
            BN_mod_mul(x2.get(), b.get(), y2, p.get(), ctx.get());

            std::vector<std::uint8_t> plaintext{};

            int size1{BN_num_bytes(x1.get())};
            std::vector<std::uint8_t> x1v{};
            x1v.resize(size1);
            BN_bn2bin(x1.get(), &x1v[0]);
            x1v = prepend_zeroes(x1v, digsiz);

            int size2{BN_num_bytes(x2.get())};
            std::vector<std::uint8_t> x2v{};
            x2v.resize(size2);
            BN_bn2bin(x2.get(), &x2v[0]);
            x2v = prepend_zeroes(x2v, digsiz);

            plaintext = x1v;
            plaintext.insert(plaintext.end(), x2v.begin(), x2v.end());
            return plaintext;
        }

        static std::vector<std::uint8_t> EC_MV_decrypt(std::vector<std::uint8_t> const &cipher, std::string const &privkey) {
            try {
                ssl_ptr<EC_KEY> ec_priv{priv_pem_to_ec_key(privkey)};
                return EC_MV_decrypt(cipher, ec_priv.get());
            } catch(...) {
            }
            return {};
        }

        static bool check_ec_pair_match(std::string const &pri, std::string const &pub) {
            bool res{false};
            try {
                auto bytes{gen_rand_bytes(64)};
                std::vector<std::uint8_t> ctrl{teal::ssl::EC_MV_encrypt(bytes, pub)};
                std::vector<std::uint8_t> ucontrol{teal::ssl::EC_MV_decrypt(ctrl, pri)};
                res = bytes == ucontrol;
            } catch(...) {
            }
            return res;
        }

        static std::vector<std::uint8_t> sign_buff_new(std::string const &priv_key_pem, unsigned char const *buff, int buff_len) {
            std::vector<std::uint8_t> ressig{};
            if(is_valid_priv_key(priv_key_pem) && buff && buff_len > 0) {
                std::array<std::uint8_t, 32> md{crypt::sha3_256_compute(buff, buff_len)};
                auto pri{teal::ssl::priv_pem_to_ec_key(priv_key_pem)};
                ssl_ptr<EVP_PKEY> key{EVP_PKEY_new()};
                if(!key) { teal::ssl::throw_last_error_if_nonzero(); }
                if(EVP_PKEY_set1_EC_KEY(key.get(), pri.get()) <= 0) { teal::ssl::throw_last_error_if_nonzero(); }
                ssl_ptr<EVP_PKEY_CTX> key_ctx{EVP_PKEY_CTX_new(key.get(), nullptr)};
                if(!key_ctx) { teal::ssl::throw_last_error_if_nonzero(); }
                if(EVP_PKEY_sign_init(key_ctx.get()) <= 0) { teal::ssl::throw_last_error_if_nonzero(); }
                if(EVP_PKEY_CTX_set_signature_md(key_ctx.get(), EVP_sha3_256()) <= 0) { teal::ssl::throw_last_error_if_nonzero(); }

                std::size_t sig_len{};
                if(EVP_PKEY_sign(key_ctx.get(), nullptr, &sig_len, md.data(), md.size()) <= 0) { teal::ssl::throw_last_error_if_nonzero(); }
                unsigned char *sig{(unsigned char *)OPENSSL_malloc(sig_len)};
                if(!sig) { teal::ssl::throw_last_error_if_nonzero(); }
                teal::shut_on_destroy freemem{[&]() { OPENSSL_free(sig); }};

                if(EVP_PKEY_sign(key_ctx.get(), (unsigned char *)&sig[0], &sig_len, md.data(), md.size()) <= 0) { teal::ssl::throw_last_error_if_nonzero(); }
                ressig.assign(sig, sig + sig_len);
            }
            return ressig;
        }

        static std::vector<std::uint8_t> sign_buff_new(std::string const &priv_key_pem, std::vector<std::uint8_t> const &buff) {
            return sign_buff_new(priv_key_pem, buff.data(), buff.size());
        }

        template<std::size_t COUNT>
        static std::vector<std::uint8_t> sign_buff_new(std::string const &priv_key_pem, std::array<std::uint8_t, COUNT> const &buff) {
            return sign_buff_new(priv_key_pem, buff.data(), buff.size());
        }

        static std::vector<std::uint8_t> sign_buff_new(std::string const &priv_key_pem, std::string const &buff) {
            return sign_buff_new(priv_key_pem, (unsigned char const *)buff.data(), buff.size());
        }

        static bool verify_sig_of_buff_new(std::string const &pub_key_pem, unsigned char const *buff, std::size_t buff_len, std::vector<std::uint8_t> const &sig) {
            bool res{false};
            if(buff && buff_len && sig.size()) {
                auto pub{teal::ssl::pub_pem_to_ec_key(pub_key_pem)};
                ssl_ptr<EVP_PKEY> key{EVP_PKEY_new()};
                if(!key) {
                    teal::ssl::throw_last_error_if_nonzero();
                }
                if(1 != EVP_PKEY_set1_EC_KEY(key.get(), pub.get())) {
                    teal::ssl::throw_last_error_if_nonzero();
                }
                ssl_ptr<EVP_PKEY_CTX> key_ctx{EVP_PKEY_CTX_new(key.get(), nullptr)};
                if(1 != EVP_PKEY_verify_init(key_ctx.get())) {
                    teal::ssl::throw_last_error_if_nonzero();
                }
                if(1 != EVP_PKEY_CTX_set_signature_md(key_ctx.get(), EVP_sha3_256())) {
                    teal::ssl::throw_last_error_if_nonzero();
                }
                std::array<std::uint8_t, 32> md{crypt::sha3_256_compute(buff, buff_len)};
                const int ret{EVP_PKEY_verify(key_ctx.get(), (unsigned char *)&sig[0], sig.size(), md.data(), md.size())};
                res = ret == 1;
            }
            return res;
        }

        static bool verify_sig_of_buff_new(std::string const &pub_key_pem, std::vector<std::uint8_t> const &buff, std::vector<std::uint8_t> const &sig) {
            return verify_sig_of_buff_new(pub_key_pem, buff.data(), buff.size(), sig);
        }

        static bool verify_sig_of_buff_new(std::string const &pub_key_pem, std::string const &buff, std::vector<std::uint8_t> const &sig) {
            return verify_sig_of_buff_new(pub_key_pem, (unsigned char const *)buff.data(), buff.size(), sig);
        }

        static std::vector<std::uint8_t> encrypt_signed_new(std::vector<std::uint8_t> const &plaintext, std::vector<std::uint8_t> const &candidate_key, std::string const &authors_priv_pem) {
            try {
                auto ph{teal::crypt::sha3_512_compute(plaintext)};
                std::vector<std::uint8_t> sig{teal::ssl::sign_buff_new(authors_priv_pem, ph)};
                if(!sig.empty()) {
                    std::vector<std::uint8_t> cipher{};
                    cipher.resize(plaintext.size());
                    std::array<std::uint8_t, 16> mac{};
                    auto iv{gen_rand_bytes(12)};
                    std::vector<std::uint8_t> key{candidate_key};
                    if(key.size() != 32) {
                        auto k256{teal::crypt::sha3_256_compute(key)};
                        key.assign(k256.begin(), k256.end());
                    }
                    if(teal::crypt::chacha20_poly1305::encrypt(key.data(), key.size(), iv.data(), iv.size(), sig.data(), sig.size(), (const uint8_t *)plaintext.data(), &cipher[0], plaintext.size(), &mac[0], mac.size())) {
                        teal::serializer sr{};
                        sr << sig << cipher << mac << iv;
                        return teal::crypt::block_container_encrypt(sr.data_vec(), candidate_key);
                    }
                }
            } catch(...) {
            }
            return {};
        }

        static std::string encrypt_signed_new(std::string const &plaintext, std::vector<std::uint8_t> const &candidate_key, std::string const &authors_priv_pem) {
            try {
                auto ph{teal::crypt::sha3_512_compute(plaintext)};
                std::vector<std::uint8_t> sig{teal::ssl::sign_buff_new(authors_priv_pem, ph)};
                if(!sig.empty()) {
                    std::vector<std::uint8_t> cipher{};
                    cipher.resize(plaintext.size());
                    std::array<std::uint8_t, 16> mac{};
                    auto iv{gen_rand_bytes(12)};
                    std::vector<std::uint8_t> key{candidate_key};
                    if(key.size() != 32) {
                        auto k256{teal::crypt::sha3_256_compute(key)};
                        key.assign(k256.begin(), k256.end());
                    }
                    if(teal::crypt::chacha20_poly1305::encrypt(key.data(), key.size(), iv.data(), iv.size(), sig.data(), sig.size(), (const uint8_t *)plaintext.data(), &cipher[0], plaintext.size(), &mac[0], mac.size())) {
                        teal::serializer sr{};
                        sr << sig << cipher << mac << iv;
                        return teal::data_to_base64_str(teal::crypt::block_container_encrypt(sr.data_vec(), candidate_key));
                    }
                }
            } catch(...) {
            }
            return {};
        }

        static std::string decrypt_signed_new(std::string const &b64_cipher, std::vector<std::uint8_t> const &candidate_key, std::string const &authors_pub_pem, bool *is_success = nullptr) {
            if(is_success) { *is_success = false; }
            try {
                if(b64_cipher.empty()) { return {}; }
                std::vector<std::uint8_t> cipher_pack{teal::crypt::block_container_decrypt(teal::base64_str_to_data(b64_cipher), candidate_key)};
                if(cipher_pack.empty()) { return {}; }
                teal::serial_reader rd{cipher_pack.data(),cipher_pack.size()};
                auto it{rd.begin()}; if(!it.valid()) { return {}; }
                std::vector<std::uint8_t> sig{it->as_bytevec()}; ++it; if(!it.valid()) { return {}; }
                if(sig.empty()) { return {}; }
                std::vector<std::uint8_t> cipher{it->as_bytevec()}; ++it; if(!it.valid()) { return {}; }
                if(cipher.empty()) { return {}; }
                std::vector<std::uint8_t> mac{it->as_bytevec()}; ++it; if(!it.valid()) { return {}; }
                if(mac.empty()) { return {}; }
                std::vector<std::uint8_t> iv{it->as_bytevec()};
                if(iv.empty()) { return {}; }
                std::vector<std::uint8_t> plaintext{};
                plaintext.resize(cipher.size());
                std::vector<std::uint8_t> key{candidate_key};
                if(key.size() != 32) {
                    auto k256{teal::crypt::sha3_256_compute(key)};
                    key.assign(k256.begin(), k256.end());
                }
                if(teal::crypt::chacha20_poly1305::decrypt(key.data(), key.size(), iv.data(), iv.size(), sig.data(), sig.size(), (const uint8_t *)cipher.data(), &plaintext[0], cipher.size(), &mac[0], mac.size())) {
                    auto ph{teal::crypt::sha3_512_compute(plaintext)};
                    if(teal::ssl::verify_sig_of_buff_new(authors_pub_pem, (const unsigned char *)ph.data(), ph.size(), sig)) {
                        if(is_success) { *is_success = true; }
                        return {plaintext.begin(), plaintext.end()};
                    }
                }
            } catch(...) {
            }
            return {};
        }

        static std::vector<std::uint8_t> decrypt_signed_new(std::vector<std::uint8_t> const &ciphertext, std::vector<std::uint8_t> const &candidate_key, std::string const &authors_pub_pem, bool *is_success = nullptr) {
            if(is_success) { *is_success = false; }
            try {
                if(ciphertext.empty()) { return {}; }
                std::vector<std::uint8_t> cipher_pack{teal::crypt::block_container_decrypt(ciphertext, candidate_key)};
                if(cipher_pack.empty()) { return {}; }
                teal::serial_reader rd{cipher_pack.data(),cipher_pack.size()};
                auto it{rd.begin()}; if(!it.valid()) { return {}; }
                std::vector<std::uint8_t> sig{it->as_bytevec()}; ++it; if(!it.valid()) { return {}; }
                if(sig.empty()) { return {}; }
                std::vector<std::uint8_t> cipher{it->as_bytevec()}; ++it; if(!it.valid()) { return {}; }
                if(cipher.empty()) { return {}; }
                std::vector<std::uint8_t> mac{it->as_bytevec()}; ++it; if(!it.valid()) { return {}; }
                if(mac.empty()) { return {}; }
                std::vector<std::uint8_t> iv{it->as_bytevec()};
                if(iv.empty()) { return {}; }
                std::vector<std::uint8_t> plaintext{};
                plaintext.resize(cipher.size());
                std::vector<std::uint8_t> key{candidate_key};
                if(key.size() != 32) {
                    auto k256{teal::crypt::sha3_256_compute(key)};
                    key.assign(k256.begin(), k256.end());
                }
                if(teal::crypt::chacha20_poly1305::decrypt(key.data(), key.size(), iv.data(), iv.size(), sig.data(), sig.size(), (const uint8_t *)cipher.data(), &plaintext[0], cipher.size(), &mac[0], mac.size())) {
                    auto ph{teal::crypt::sha3_512_compute(plaintext)};
                    if(teal::ssl::verify_sig_of_buff_new(authors_pub_pem, (const unsigned char *)ph.data(), ph.size(), sig)) {
                        if(is_success) { *is_success = true; }
                        return plaintext;
                    }
                }
            } catch(...) {
            }
            return {};
        }

        class crypto_store {
        public:
            void set_data(std::vector<std::uint8_t> const &d) {
                d_.clear();
                d_.reserve(d.size());
                for(std::size_t pos{0}; pos < d.size(); ++pos) {
                    d_.push_back(d[pos] ^ k_[pos % k_.size()]);
                }
            }

            void set_data(std::string const &d) {
                set_data(std::vector<std::uint8_t>{d.begin(), d.end()});
            }

            crypto_wrapper<std::vector<std::uint8_t>> get_as_bytevec() const {
                crypto_wrapper<std::vector<std::uint8_t>> res{};
                res->reserve(d_.size());
                for(std::size_t pos{0}; pos < d_.size(); ++pos) {
                    res->push_back(d_[pos] ^ k_[pos % k_.size()]);
                }
                return res;
            }

            crypto_wrapper<std::string> get_as_string() const {
                crypto_wrapper<std::vector<std::uint8_t>> v{get_as_bytevec()};
                return crypto_wrapper<std::string>{std::string{v->begin(), v->end()}};
            }

        private:
            std::vector<std::uint8_t> d_{};
            std::vector<std::uint8_t> k_{gen_rand_bytes(64)};
        };

    }

}
