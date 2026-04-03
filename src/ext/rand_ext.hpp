#pragma once

#include "../inc/commondefs.hpp"

#include "../tealscript_value.hpp"
#include "../tealscript_util.hpp"
#include "../tealscript_interfaces.hpp"

namespace teal {

    class rand_ext: public extension_interface {
        template<typename T>
        class norm_dist_gen {
        public:
            norm_dist_gen(T mean = 0, T stddev = 1): nrd_{mean, stddev} {}
            T gen() { return nrd_(dre_); }
            T operator()() { return gen(); }

        private:
            T mean_;
            T dis_;
            std::random_device rd_{};
            std::default_random_engine dre_{rd_()};
            std::normal_distribution<T> nrd_;
        };

    public:
        rand_ext() = default;
        ~rand_ext() {
            unregister_runtime();
        }
        rand_ext(rand_ext const &) = delete;
        rand_ext &operator=(rand_ext const &) = delete;
        rand_ext(rand_ext &&) = delete;
        rand_ext &operator=(rand_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt->add_function("normal_dist_prng", TEALFUN(args) {
                if(args.size() == 1) {
                    return teal::valbox{std::make_shared<norm_dist_gen<long double>>(args[0].cast_to_long_double()), "normal_dist_prng"};
                } else if(args.size() == 2) {
                    return teal::valbox{std::make_shared<norm_dist_gen<long double>>(args[0].cast_to_long_double(), args[1].cast_to_long_double()), "normal_dist_prng"};
                }
                return teal::valbox{std::make_shared<norm_dist_gen<long double>>(), "normal_dist_prng"};
            });
            rt->add_method("normal_dist_prng", "gen", TEALFUN(args) {
                if(args.size() == 1) {
                    return TEALTHIS(args, std::shared_ptr<norm_dist_gen<long double>>)->gen();
                }
                return static_cast<long double>(0);
            });
            rt->add_function("rand", TEALFUN() { return ud_(dre_); });
            rt->add_function("hwrand", TEALFUN() { return ud_(rd_); });
            rt->add_function("randf", TEALFUN() { return urd_(dre_); });
            rt->add_function("hwrandf", TEALFUN() { return urd_(rd_); });
            rt->add_function("frand", TEALFUN() { return urd_(dre_); });
            rt->add_function("hw_frand", TEALFUN() { return urd_(rd_); });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("normal_dist_prng");
            rt_->remove_method("normal_dist_prng", "gen");
            rt_->remove_function("rand");
            rt_->remove_function("hwrand");
            rt_->remove_function("randf");
            rt_->remove_function("hwrandf");
            rt_->remove_function("frand");
            rt_->remove_function("hw_frand");
            rt_ = nullptr;
        }

    private:
        std::random_device rd_{};
        std::mt19937_64 dre_{rd_()};
        std::uniform_int_distribution<std::uint64_t> ud_{};
        std::uniform_real_distribution<long double> urd_{0.0L, 1.0L};

        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
