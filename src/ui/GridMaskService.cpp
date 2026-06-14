#include "ui/GridMaskService.h"

#include "data/StaticDataLoader.h"

#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

namespace rc {
namespace GridMaskService {

namespace {

constexpr std::array<const char*, 7> kMaskFiles = {
    "mask_01.png", "mask_02.png", "mask_03.png", "mask_04.png",
    "mask_05.png", "mask_06.png", "mask_07.png",
};

AddonAPI_t* g_api = nullptr;
std::string g_addonDir;
std::vector<ImTextureID> g_masks;
std::atomic<bool> g_downloadInFlight{false};

OrganicCellStyle BuildStyle(uint32_t seed) {
    OrganicCellStyle style;
    if (g_masks.empty()) return style;

    style.texture = g_masks[seed % g_masks.size()];
    style.xOffset = static_cast<float>(seed % 3);
    style.yOffset = static_cast<float>((seed / 3) % 2);
    style.widthDelta = static_cast<float>(-1 - static_cast<int>((seed / 6) % 3));
    style.heightDelta = static_cast<float>(-1 - static_cast<int>((seed / 18) % 2));
    return style;
}

std::filesystem::path MaskDir() {
    return std::filesystem::path(g_addonDir) / "textures" / "masks";
}

std::vector<std::string> LoadMaskManifest() {
    std::vector<std::string> masks;
    const auto manifestPath =
        std::filesystem::path(g_addonDir) / StaticDataLoader::kCacheDirName / "clears_tracker.json";
    if (std::filesystem::exists(manifestPath)) {
        try {
            std::ifstream in(manifestPath);
            nlohmann::json j;
            in >> j;
            if (j.contains("gridbox_masks") && j["gridbox_masks"].is_array()) {
                for (const auto& mask : j["gridbox_masks"]) {
                    if (mask.is_string()) {
                        masks.push_back(mask.get<std::string>());
                    }
                }
            }
        } catch (...) {
        }
    }

    if (masks.empty()) {
        masks.reserve(kMaskFiles.size());
        for (const char* mask : kMaskFiles) {
            masks.emplace_back(mask);
        }
    }
    return masks;
}

bool AnyMasksMissing(const std::vector<std::string>& masks) {
    const auto maskDir = MaskDir();
    for (const auto& mask : masks) {
        if (!std::filesystem::exists(maskDir / mask)) {
            return true;
        }
    }
    return false;
}

void LoadMaskTextures() {
    g_masks.clear();
    if (!g_api || !g_api->Textures_GetOrCreateFromFile) return;

    const auto maskDir = MaskDir();
    const auto manifest = LoadMaskManifest();
    for (size_t i = 0; i < manifest.size(); ++i) {
        const auto path = (maskDir / manifest[i]).string();
        if (!std::filesystem::exists(path)) continue;

        const std::string identifier = "NRC_MASK_" + manifest[i];
        Texture_t* texture = g_api->Textures_GetOrCreateFromFile(identifier.c_str(), path.c_str());
        if (texture && texture->Resource) {
            g_masks.push_back(reinterpret_cast<ImTextureID>(texture->Resource));
        }
    }
}

void DownloadMasksWorker(std::vector<std::string> masks) {
    const auto maskDir = MaskDir();
    std::error_code ec;
    std::filesystem::create_directories(maskDir, ec);

    for (const auto& mask : masks) {
        const auto dest = maskDir / mask;
        if (std::filesystem::exists(dest)) continue;
        StaticDataLoader::DownloadToPath(dest, mask);
    }

    LoadMaskTextures();
    g_downloadInFlight.store(false);
}

}  // namespace

void Initialize(AddonAPI_t* api, const std::string& addonDir) {
    g_api = api;
    g_addonDir = addonDir;
    LoadMaskTextures();
}

void RequestMasks() {
    if (!g_api || g_addonDir.empty()) return;

    LoadMaskTextures();

    const auto masks = LoadMaskManifest();
    if (!AnyMasksMissing(masks)) return;
    if (g_downloadInFlight.exchange(true)) return;

    std::thread(DownloadMasksWorker, masks).detach();
}

bool HasMasks() { return !g_masks.empty(); }

OrganicCellStyle StyleForSeed(uint32_t seed) { return BuildStyle(seed); }

uint32_t HashSeed(const std::string& groupId, const std::string& cellId) {
    std::hash<std::string> hasher;
    if (cellId.empty()) {
        return static_cast<uint32_t>(hasher(groupId));
    }
    return static_cast<uint32_t>(hasher(groupId + ":" + cellId));
}

}  // namespace GridMaskService
}  // namespace rc
