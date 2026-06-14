#pragma once

#include <cstring>
#include <string>

namespace rc {

constexpr const char* kPriorityPrefix = "priority_";
constexpr const char* kTomorrowPrefix = "tomorrow_";

inline std::string NormalizeStorageKey(const std::string& key) {
    if (key == "priority" || key == "priority_tomorrow") return key;
    if (key.rfind(kPriorityPrefix, 0) == 0) {
        return key.substr(std::strlen(kPriorityPrefix));
    }
    if (key.rfind(kTomorrowPrefix, 0) == 0) {
        return key.substr(std::strlen(kTomorrowPrefix));
    }
    return key;
}

}  // namespace rc
