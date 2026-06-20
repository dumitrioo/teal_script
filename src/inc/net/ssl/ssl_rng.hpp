#pragma once

#include "../../commondefs.hpp"
#include "../../bit_util.hpp"
#include "ssl_str.hpp"

#include <openssl/rand.h>

namespace teal {

    namespace ssl {

        namespace rand {

            static inline std::mutex rng_mtp{};

            static teal::ssl::bytevec gen_rand_bytes(std::size_t num_bytes) {
                teal::ssl::bytevec res(num_bytes);
                if(num_bytes > 0) {
                    std::lock_guard lck{rng_mtp};
                    /*
                    RAND_bytes() generates num random bytes using a cryptographically secure
                    pseudo random generator (CSPRNG) and stores them in buf.
                    */
                    RAND_bytes(&res[0], num_bytes);
                }
                return res;
            }

            static teal::ssl::bytevec gen_rand_priv_bytes(std::size_t num_bytes) {
                teal::ssl::bytevec res(num_bytes);
                if(num_bytes > 0) {
                    std::lock_guard lck{rng_mtp};
                    /*
                    RAND_priv_bytes() has the same semantics as RAND_bytes(). It is intended
                    to be used for generating values that should remain private. If using the
                    default RAND_METHOD, this function uses a separate "private" PRNG instance
                    so that a compromise of the "public" PRNG instance will not affect the
                    secrecy of these private values, as described in RAND(7) and RAND_DRBG(7).
                    */
                    RAND_priv_bytes(&res[0], num_bytes);
                }
                return res;
            }

            class random_device {
            public:
                typedef unsigned int result_type;

                explicit random_device(const std::string& __token) {
                    RAND_seed(__token.data(), __token.size());
                }

                random_device() = default;
                random_device(const random_device &) = delete;
                random_device &operator=(const random_device &) = delete;
                random_device(random_device &&) = default;
                random_device &operator=(random_device &&) = default;
                ~random_device() {}

                static constexpr result_type
                min() { return std::numeric_limits<result_type>::min(); }

                static constexpr result_type
                max() { return std::numeric_limits<result_type>::max(); }

                double entropy() const noexcept {
                    return 0;
                }

                result_type operator()() {
                    result_type res{};
                    RAND_bytes((unsigned char *)&res, sizeof(result_type));
                    return teal::bit_util::swap_on_be{res}.val;
                }

            };

        }

    }
}
