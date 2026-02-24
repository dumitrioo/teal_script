#pragma once

#include "commondefs.hpp"

namespace scfx {

    template<typename T>
    class sequence_generator {
    public:
        sequence_generator(T starting = 1, T omitted = 0, std::function<T(T const &)> operation = [](T const &v) { return v + 1; }):
            stored_{starting},
            omitted_{omitted},
            operation_{operation}
        {
        }

        T next() {
            T res{stored_};
            do {
                if(operation_) {
                    stored_ = operation_(stored_);
                } else {
                    ++stored_;
                }
            } while(stored_ == omitted_ || res == omitted_);
            return res;
        }

        T curr() const {
            return stored_;
        }

        operator T() {
            return next();
        }

        T operator()() {
            return next();
        }

        void reset(T val = 1) {
            stored_ = val;
        }

    private:
        T stored_{1};
        T omitted_{0};
        std::function<T(T const &)> operation_{nullptr};
    };


    template<typename T>
    class atomic_sequence_generator {
    public:
        atomic_sequence_generator(T starting = 1, T omitted = 0):
            stored_{starting},
            omitted_{omitted}
        {
        }

        T next() {
            T res{std::atomic_fetch_add(&stored_, static_cast<T>(1))};
            while(res == omitted_) {
                res = std::atomic_fetch_add(&stored_, static_cast<T>(1));
            }
            return res;
        }

        T curr() const {
            return stored_.load();
        }

        operator T() {
            return next();
        }

        T operator()() {
            return next();
        }

        void reset(T val = 1, T omitted = 0) {
            stored_.store(val);
            omitted_ = omitted;
        }

    private:
        std::atomic<T> stored_{1};
        T omitted_{0};
    };

}
