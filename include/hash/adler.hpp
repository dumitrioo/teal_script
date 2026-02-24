#pragma once

#include "../commondefs.hpp"

namespace scfx {

    static std::uint32_t adler32(void const *buf, std::size_t buflength) {
        std::uint32_t s1{1};
        std::uint32_t s2{0};

        for(std::size_t n{0}; n < buflength; ++n) {
            s1 = (s1 + (reinterpret_cast<std::uint8_t const *>(buf))[n]) % 65521;
            s2 = (s2 + s1) % 65521;
        }
        return (s2 << 16) + s1;
    }

}
