#pragma once

#include "../commondefs.hpp"
#include "sha3_256.hpp"
#include "chacha.hpp"

namespace teal::crypt {

    class chacha_stream {
    public:
        chacha_stream() = default;
        chacha_stream(chacha_stream const &) = default;
        chacha_stream(chacha_stream &&) = default;
        chacha_stream &operator=(chacha_stream const &) = default;
        chacha_stream &operator=(chacha_stream &&) = default;
        ~chacha_stream() = default;

        chacha_stream(std::string const &key, std::string const &nonce) {
            reset(key, nonce);
        }

        chacha_stream(std::vector<std::uint8_t> const &key, std::vector<std::uint8_t> const &nonce) {
            reset(key, nonce);
        }

        void reset(std::vector<std::uint8_t> const &key, std::vector<std::uint8_t> const &nonce, std::uint32_t rnds = 20) {
            if(rnds != 8 && rnds != 12 && rnds != 20) {
                rnds = 20;
            }
            if(key.size() == 32 || key.size() == 16) {
                if(nonce.size() == 12 || nonce.size() == 8) {
                    state_.init(rnds, key.data(), key.size(), nonce.data(), nonce.size());
                } else {
                    std::array<std::uint8_t, 32> right_nonce{sha3_256_compute(nonce)};
                    state_.init(rnds, key.data(), key.size(), right_nonce.data(), 8);
                }
            } else {
                std::array<std::uint8_t, 32> right_key{sha3_256_compute(key)};
                if(nonce.size() == 12 || nonce.size() == 8) {
                    state_.init(rnds, right_key.data(), right_key.size(), nonce.data(), nonce.size());
                } else {
                    std::array<std::uint8_t, 32> right_nonce{sha3_256_compute(nonce)};
                    state_.init(rnds, right_key.data(), right_key.size(), right_nonce.data(), 8);
                }
            }
        }

        void reset(std::string const &key, std::string const &nonce) {
            reset(
                std::vector<std::uint8_t>{key.begin(), key.end()},
                std::vector<std::uint8_t>{nonce.begin(), nonce.end()}
            );
        }

        std::vector<std::uint8_t> cipher(void const *in, std::size_t len) {
            std::vector<std::uint8_t> res(len);
            if(len > 0) {
                state_.cipher((std::uint8_t const *)in, &res[0], len);
            }
            return res;
        }

        std::vector<std::uint8_t> cipher(std::vector<std::uint8_t> const &some_data) {
            return cipher(some_data.data(), some_data.size());
        }

        std::vector<std::uint8_t> cipher(std::string const &some_data) {
            return cipher(some_data.data(), some_data.size());
        }

    private:
        teal::crypt::chacha state_{};
    };

}
