#include "core/Types.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace rc {

ColorRGB ColorRGB::FromHex(const std::string& hex) {
    std::string s = hex;
    if (!s.empty() && s[0] == '#') {
        s = s.substr(1);
    }
    if (s.size() != 6) {
        return {128, 128, 128};
    }
    auto parse = [](char a, char b) -> uint8_t {
        auto nybble = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return 0;
        };
        return static_cast<uint8_t>((nybble(a) << 4) | nybble(b));
    };
    return {parse(s[0], s[1]), parse(s[2], s[3]), parse(s[4], s[5])};
}

uint32_t ColorRGB::ToImU32(float alpha) const {
    const auto a = static_cast<uint32_t>(alpha * 255.0f) & 0xFF;
    return (a << 24) | (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(r);
}

}  // namespace rc
