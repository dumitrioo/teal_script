#pragma once

#include "../../commondefs.hpp"
#include "../../str_util.hpp"
#include "../../base16.hpp"

#include <openssl/evp.h>
#include <openssl/crypto.h>

namespace teal {

    namespace ssl {

        template <typename T>
        struct allocator: public std::allocator<T> {
        public:
            using pointer = typename std::allocator<T>::pointer;
            using size_type = typename std::allocator<T>::size_type;

            void deallocate(pointer p, size_type n) {
                OPENSSL_cleanse(p, n * sizeof(T));
                std::allocator<T>::deallocate(p, n);
            }

            template<typename U>
            struct rebind {
                using other = allocator<U>;
            };
        };

        using bytevec = std::vector<std::uint8_t>;

        static std::string data_to_hex_str(bytevec const &bv) {
            auto tmp_res{teal::data_to_hex_str(bv.data(), bv.size())};
            return {tmp_res.begin(), tmp_res.end()};
        }

        static std::string data_to_hex_str(void const *data, std::size_t size) {
            auto tmp_res{teal::data_to_hex_str(data, size)};
            return {tmp_res.begin(), tmp_res.end()};
        }

         static bytevec hex_str_to_data(std::string const &hs) {
            auto tmp_res{teal::hex_str_to_data({hs.begin(), hs.end()})};
            return {tmp_res.begin(), tmp_res.end()};
        }

        static std::ostream &operator<<(std::ostream &os, std::string const &s) {
            for(auto &&c: s) { os << c; }
            return os;
        }

        static std::ostream &operator<<(std::ostream &os, bytevec const &v) {
            os << data_to_hex_str(v);
            return os;
        }

    }
    
}

