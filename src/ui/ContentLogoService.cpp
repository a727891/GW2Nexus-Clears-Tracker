#include "ui/ContentLogoService.h"

#include "EmbeddedLogos.h"

#include <cstring>
#include <filesystem>
#include <mutex>
#include <unordered_map>

namespace rc {
namespace ContentLogoService {
namespace {

constexpr float kDisplayHeight = 102.0f;
constexpr float kFallbackAspect = 195.0f / 153.0f;

AddonAPI_t* g_api = nullptr;
std::string g_addonDir;
std::mutex g_mutex;
std::unordered_map<std::string, LogoInfo> g_logos;

const char* LogoFilenameForExpansion(const char* expansionId) {
    if (!expansionId) return nullptr;

    static const std::unordered_map<std::string, const char*> kMap = {
        {"hot", "heart_of_thorns.png"},
        {"pof", "path_of_fire.png"},
        {"janthir_wilds", "janthir_wilds.png"},
        {"ibs", "ice_brood_saga.png"},
        {"eod", "end_of_dragons.png"},
        {"soto", "secrets_of_the_obscure.png"},
        {"voe", "visions_of_eternity.png"},
        {"core", "guild_war_2.png"},
    };

    const auto it = kMap.find(expansionId);
    return it != kMap.end() ? it->second : nullptr;
}

std::filesystem::path LogoDir() {
    return std::filesystem::path(g_addonDir) / "textures" / "logos";
}

LogoInfo LogoFromMemory(const EmbeddedLogos::Asset& asset) {
    LogoInfo info;
    if (!g_api || !g_api->Textures_GetOrCreateFromMemory) return info;

    Texture_t* texture = g_api->Textures_GetOrCreateFromMemory(
        asset.identifier, const_cast<unsigned char*>(asset.data), asset.size);
    if (!texture || !texture->Resource) return info;

    info.texture = reinterpret_cast<ImTextureID>(texture->Resource);
    info.width = texture->Width;
    info.height = texture->Height;
    return info;
}

LogoInfo LogoFromFile(const std::string& filename) {
    LogoInfo info;
    if (!g_api || !g_api->Textures_GetOrCreateFromFile) return info;

    const auto path = (LogoDir() / filename).string();
    if (!std::filesystem::exists(path)) return info;

    const std::string identifier = "NRC_LOGO_FILE_" + filename;
    Texture_t* texture = g_api->Textures_GetOrCreateFromFile(identifier.c_str(), path.c_str());
    if (!texture || !texture->Resource) return info;

    info.texture = reinterpret_cast<ImTextureID>(texture->Resource);
    info.width = texture->Width;
    info.height = texture->Height;
    return info;
}

LogoInfo LoadLogo(const char* expansionId) {
    if (!expansionId) return {};

    const std::string cacheKey = expansionId;
    {
        std::lock_guard lock(g_mutex);
        const auto it = g_logos.find(cacheKey);
        if (it != g_logos.end() && it->second.texture != nullptr) {
            return it->second;
        }
    }

    if (const EmbeddedLogos::Asset* asset = EmbeddedLogos::Find(expansionId)) {
        if (auto info = LogoFromMemory(*asset); info.texture) {
            std::lock_guard lock(g_mutex);
            g_logos[cacheKey] = info;
            return info;
        }
    }

    const char* filename = LogoFilenameForExpansion(expansionId);
    if (filename) {
        if (auto info = LogoFromFile(filename); info.texture) {
            std::lock_guard lock(g_mutex);
            g_logos[cacheKey] = info;
            return info;
        }
    }

    return {};
}

ImVec2 LogoDisplaySize(const LogoInfo& info) {
    const float aspect = info.width > 0 && info.height > 0
                             ? static_cast<float>(info.width) /
                                   static_cast<float>(info.height)
                             : kFallbackAspect;
    return ImVec2(kDisplayHeight * aspect, kDisplayHeight);
}

}  // namespace

void Initialize(AddonAPI_t* api, const std::string& addonDir) {
    g_api = api;
    g_addonDir = addonDir;
}

LogoInfo GetLogo(const char* expansionId) { return LoadLogo(expansionId); }

bool RenderLogo(const char* expansionId) {
    const auto info = LoadLogo(expansionId);
    if (!info.texture) return false;

    ImGui::Image(info.texture, LogoDisplaySize(info));
    return true;
}

}  // namespace ContentLogoService
}  // namespace rc
