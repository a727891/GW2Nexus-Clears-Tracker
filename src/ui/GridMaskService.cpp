#include "ui/GridMaskService.h"

#include <array>
#include <filesystem>
#include <vector>

namespace rc {
namespace GridMaskService {

namespace {

constexpr std::array<const char*, 7> kMaskFiles = {
    "mask_01.png", "mask_02.png", "mask_03.png", "mask_04.png",
    "mask_05.png", "mask_06.png", "mask_07.png",
};

std::vector<ImTextureID> g_masks;

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

}  // namespace

void Initialize(AddonAPI_t* api, const std::string& addonDir) {
    g_masks.clear();
    if (!api || !api->Textures_GetOrCreateFromFile) return;

    const auto maskDir = std::filesystem::path(addonDir) / "textures" / "masks";
    for (size_t i = 0; i < kMaskFiles.size(); ++i) {
        const auto path = (maskDir / kMaskFiles[i]).string();
        if (!std::filesystem::exists(path)) continue;

        const std::string identifier = "NRC_MASK_" + std::to_string(i);
        Texture_t* texture = api->Textures_GetOrCreateFromFile(identifier.c_str(), path.c_str());
        if (texture && texture->Resource) {
            g_masks.push_back(reinterpret_cast<ImTextureID>(texture->Resource));
        }
    }
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
