#include "ui/GridMaskService.h"

#include "EmbeddedMasks.h"
#include "data/StaticDataLoader.h"

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

AddonAPI_t* g_api = nullptr;
std::string g_addonDir;
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

std::filesystem::path CacheDir() {
    return std::filesystem::path(g_addonDir) / StaticDataLoader::kCacheDirName;
}

bool LoadMaskFromMemory(const EmbeddedMasks::Asset& asset) {
    if (!g_api || !g_api->Textures_GetOrCreateFromMemory) return false;

    Texture_t* texture = g_api->Textures_GetOrCreateFromMemory(
        asset.identifier, const_cast<unsigned char*>(asset.data), asset.size);
    if (!texture || !texture->Resource) return false;

    g_masks.push_back(reinterpret_cast<ImTextureID>(texture->Resource));
    return true;
}

bool LoadMaskFromFile(const char* filename) {
    if (!g_api || !g_api->Textures_GetOrCreateFromFile) return false;

    const auto path = (CacheDir() / filename).string();
    if (!std::filesystem::exists(path)) return false;

    const std::string identifier = "CLRTRK_MASK_FILE_" + std::string(filename);
    Texture_t* texture = g_api->Textures_GetOrCreateFromFile(identifier.c_str(), path.c_str());
    if (!texture || !texture->Resource) return false;

    g_masks.push_back(reinterpret_cast<ImTextureID>(texture->Resource));
    return true;
}

void LoadMaskTextures() {
    g_masks.clear();
    if (!g_api) return;

    for (std::size_t i = 0; i < EmbeddedMasks::Count(); ++i) {
        const EmbeddedMasks::Asset* asset = EmbeddedMasks::At(i);
        if (!asset) continue;
        LoadMaskFromMemory(*asset);
    }

    if (!g_masks.empty()) return;

    for (const char* filename : kMaskFiles) {
        LoadMaskFromFile(filename);
    }
}

}  // namespace

void Initialize(AddonAPI_t* api, const std::string& addonDir) {
    g_api = api;
    g_addonDir = addonDir;
}

void RequestMasks() { LoadMaskTextures(); }

void Shutdown() {
    g_masks.clear();
    g_api = nullptr;
    g_addonDir.clear();
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
