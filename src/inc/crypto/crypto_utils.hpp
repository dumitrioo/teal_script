#pragma once

#include "../commondefs.hpp"
#include "../serialization.hpp"
#include "../memory_file.hpp"
#include "chacha20_poly1305.hpp"
#include "chacha_stream.hpp"
#include "threefish.hpp"
#include "sha256.hpp"
#include "sha512.hpp"
#include "sha3_256.hpp"
#include "sha3_384.hpp"
#include "sha3_512.hpp"
#include "encrypted_stream.hpp"

namespace teal {

    namespace crypt {

        static std::vector<std::uint8_t> gen_rand_bytes(std::size_t num_bytes) {
            std::vector<std::uint8_t> res(num_bytes);
            if(num_bytes > 0) {
                static std::mutex rd_mtp{};
                static std::random_device rd{};
                std::random_device::result_type rnum{};
                std::lock_guard l{rd_mtp};
                for(std::size_t i = 0; i < num_bytes; ++i) {
                    if(i % sizeof(std::random_device::result_type) == 0) {
                        rnum = rd();
                    }
                    res[i] = ((std::uint8_t *)&rnum)[i % 4];
                }
            }
            return res;
        }

        static std::vector<std::uint8_t> gen_zero_bytes(std::size_t num_bytes) {
            std::vector<std::uint8_t> res(num_bytes);
            return res;
        }

        static std::vector<std::uint8_t> block_container_encrypt(void const *data, std::size_t data_size, std::vector<std::uint8_t> const &candidate_key) {
            if(!data || !data_size) { return {}; }
            try {
                std::vector<std::uint8_t> key{candidate_key};
                if(key.size() != teal::crypt::threefish<512>::key_length()) {
                    auto k512{teal::crypt::sha3_512_compute(key)};
                    key.assign(k512.begin(), k512.end());
                }
                teal::crypt::encrypted_stream<teal::memory_file, teal::crypt::threefish<512>> a;
                a.set_key(key.data(), key.size());
                if(a.open() && a.reset_all_data() && a.write(data, data_size) == static_cast<std::int64_t>(data_size)) {
                    return a.file_object().get_immediate_data();
                }
            } catch(...) {
            }
            return {};
        }

        static std::vector<std::uint8_t> block_container_encrypt(std::vector<std::uint8_t> const &data, std::vector<std::uint8_t> const &key) {
            return block_container_encrypt(data.data(), data.size(), key);
        }

        template<size_t N>
        std::vector<std::uint8_t> block_container_encrypt(std::vector<std::uint8_t> const &data, std::array<std::uint8_t, N> const &key) {
            return block_container_encrypt(data.data(), data.size(), std::vector<std::uint8_t>{key.begin(), key.end()});
        }


        static std::vector<std::uint8_t> block_container_decrypt(void const *data, std::size_t data_size, std::vector<std::uint8_t> const &candidate_key) {
            if(!data || !data_size) { return {}; }
            try {
                std::vector<std::uint8_t> key{candidate_key};
                if(key.size() != teal::crypt::threefish<512>::key_length()) {
                    auto k512{teal::crypt::sha3_512_compute(key)};
                    key.assign(k512.begin(), k512.end());
                }
                teal::crypt::encrypted_stream<teal::memory_file, teal::crypt::threefish<512>> a;
                a.set_key(key.data(), key.size());
                a.file_object().set_immediate_data(std::vector<std::uint8_t>{(std::uint8_t const *)data, (std::uint8_t const *)data + data_size});
                if(a.open() && a.is_valid() && a.size() > 0) {
                    std::vector<std::uint8_t> res{};
                    res.resize(a.size());
                    a.seek(0);
                    if(a.read(&res[0], res.size()) == (std::int64_t)res.size()) {
                        return res;
                    }
                }
            } catch(...) {
            }
            return {};
        }

        static std::vector<std::uint8_t> block_container_decrypt(std::vector<std::uint8_t> const &data, std::vector<std::uint8_t> const &key) {
            return block_container_decrypt(data.data(), data.size(), key);
        }




        static std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>> chacha20_poly1305_encrypt(std::vector<std::uint8_t> const &plain_text, std::vector<std::uint8_t> const &auth_data, std::vector<std::uint8_t> const &key_candidate, std::vector<std::uint8_t> const &nonce) {
            std::vector<std::uint8_t> cipher_text{};
            cipher_text.resize(plain_text.size());
            std::array<std::uint8_t, 16> mac{};
            auto key{key_candidate};
            if(key.size() != 32) {
                auto kh{teal::crypt::sha3_256_compute(key)};
                key.assign(kh.begin(), kh.end());
            }
            teal::crypt::chacha20_poly1305::encrypt(key.data(), key.size(), nonce.data(), nonce.size(), auth_data.size() ? auth_data.data() : nullptr, auth_data.size(), (const uint8_t *)plain_text.data(), &cipher_text[0], plain_text.size(), &mac[0], mac.size());
            return {cipher_text, {mac.begin(), mac.end()}};
        }

        static std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>> chacha20_poly1305_encrypt(std::string const &plain_text, std::string const &auth_data, std::vector<std::uint8_t> const &key, std::vector<std::uint8_t> const &nonce) {
            return chacha20_poly1305_encrypt(std::vector<std::uint8_t>{plain_text.begin(), plain_text.end()}, std::vector<std::uint8_t>{auth_data.begin(), auth_data.end()}, key, nonce);
        }

        static std::vector<std::uint8_t> chacha20_poly1305_decrypt(std::vector<std::uint8_t> const &cipher_text, std::vector<std::uint8_t> const &auth_data, std::vector<std::uint8_t> const &key_candidate, std::vector<std::uint8_t> const &nonce, std::vector<std::uint8_t> const &mac) {
            std::vector<std::uint8_t> plain_text{};
            plain_text.resize(cipher_text.size());
            auto key{key_candidate};
            if(key.size() != 32) {
                auto kh{teal::crypt::sha3_256_compute(key)};
                key.assign(kh.begin(), kh.end());
            }
            if(teal::crypt::chacha20_poly1305::decrypt(key.data(), key.size(), nonce.data(), nonce.size(), auth_data.size() ? auth_data.data() : nullptr, auth_data.size(), cipher_text.data(), &plain_text[0], cipher_text.size(), &mac[0], mac.size())) {
                return plain_text;
            }
            return {};
        }

        class crypto_chunk {
        public:
            static std::vector<std::uint8_t> encrypt(std::vector<std::uint8_t> const &val, std::vector<std::uint8_t> const &key) {
                return chacha_stream{key, {}}.cipher((serializer{} << val).data_vec());
            }

            static std::vector<std::uint8_t> encrypt(std::string const &val, std::vector<std::uint8_t> const &key) {
                return encrypt(std::vector<std::uint8_t>{val.begin(), val.end()}, key);
            }

            static std::vector<std::uint8_t> decrypt(std::vector<std::uint8_t> const &val, std::vector<std::uint8_t> const &key) {
                std::vector<std::uint8_t> dec_vec{chacha_stream{key, {}}.cipher(val)};
                return serial_reader{dec_vec.data(), dec_vec.size()}.begin()->as_bytevec();
            }

            static std::string decrypt_as_string(std::vector<std::uint8_t> const &val, std::vector<std::uint8_t> const &key) {
                std::vector<std::uint8_t> dec_vec{chacha_stream{key, {}}.cipher(val)};
                return std::string{serial_reader{dec_vec.data(), dec_vec.size()}.begin()->as_string()};
            }
        };

    }

}
