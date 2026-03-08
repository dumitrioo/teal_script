#pragma once

#include "../include/commondefs.hpp"
#include "../include/crypto/sha256.hpp"
#include "../include/crypto/sha512.hpp"

#include "../scaflux_value.hpp"
#include "../scaflux_util.hpp"
#include "../scaflux_interfaces.hpp"

namespace scfx {

    class crypto_ext: public extension_interface {
    public:
        crypto_ext() = default;
        ~crypto_ext() {
            unregister_runtime();
        }
        crypto_ext(crypto_ext const &) = delete;
        crypto_ext &operator=(crypto_ext const &) = delete;
        crypto_ext(crypto_ext &&) = delete;
        crypto_ext &operator=(crypto_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt->add_function("sha256", SCFXFUN(args) {
                if(args.size() == 1) {
                    std::string s{args[0].cast_to_string()};
                    std::array<std::uint8_t, scfx::crypt::sha256::DIGEST_SIZE> h{scfx::crypt::sha256sum(s.data(), s.size())};
                    return scfx::valbox{std::string{h.begin(), h.end()}};
                }
                return scfx::valbox{std::string{}};
            });
            rt->add_function("sha512", SCFXFUN(args) {
                if(args.size() == 1) {
                    std::string s{args[0].cast_to_string()};
                    std::array<std::uint8_t, scfx::crypt::sha512::digest_size()> h{scfx::crypt::sha512sum(s.data(), s.size())};
                    return scfx::valbox{std::string{h.begin(), h.end()}};
                }
                return scfx::valbox{std::string{}};
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("sha256");
            rt_->remove_function("sha512");
            rt_ = nullptr;
        }

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
