#pragma once

#include "commondefs.hpp"

namespace teal {

    class terminable {
    public:
        void terminate() noexcept {
            termination_requested_ = 1;
        }

        void unterminate() noexcept {
            termination_requested_ = 0;
        }

        bool termination() const noexcept {
            return termination_requested_ != 0;
        }

    protected:
        std::size_t termination_requested_{0};
    };

}
