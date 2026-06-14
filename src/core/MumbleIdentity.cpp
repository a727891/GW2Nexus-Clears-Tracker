#include "core/MumbleIdentity.h"

#include <nlohmann/json.hpp>
#include <windows.h>

namespace rc {
namespace MumbleIdentity {

std::string WideToUtf8(const wchar_t* wide) {
    if (!wide || wide[0] == L'\0') return {};

    int size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};

    std::string result(static_cast<size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, result.data(), size, nullptr, nullptr);
    return result;
}

std::string ParseIdentityName(const Mumble::Data* data) {
    if (!data) return {};

    const std::string identity = WideToUtf8(data->LinkedMemory.identity);
    if (identity.empty()) return {};

    try {
        const auto j = nlohmann::json::parse(identity);
        return j.value("name", "");
    } catch (...) {
        return {};
    }
}

}  // namespace MumbleIdentity
}  // namespace rc
