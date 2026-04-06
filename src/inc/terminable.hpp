#pragma once

#include "commondefs.hpp"

namespace teal {

    class terminable {
    public:
        void terminate() noexcept {
            termination_requested_.store(1, std::memory_order_release);
        }

        void unterminate() noexcept {
            termination_requested_.store(0, std::memory_order_release);
        }

        bool termination() const noexcept {
            return termination_requested_.load(std::memory_order_acquire) != 0;
        }

    protected:
        std::atomic<std::size_t> termination_requested_{0};
    };

}
