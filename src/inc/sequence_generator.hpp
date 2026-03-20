#pragma once

#include "commondefs.hpp"

namespace scfx {

    template<typename T>
    class sequence_generator {
    public:
        sequence_generator(T starting = 0):
            stored_{starting}
        {
        }

        T next() {
            return stored_++;
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

        void reset(T val = 0) {
            stored_ = val;
        }

    private:
        T stored_{0};
    };


    template<typename T>
    class atomic_sequence_generator {
    public:
        atomic_sequence_generator(T starting = 0):
            stored_{starting}
        {
        }

        T next() {
            return std::atomic_fetch_add(&stored_, static_cast<T>(1));
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

        void reset(T val = 0) {
            stored_.store(val);
        }

    private:
        std::atomic<T> stored_{0};
    };

}
