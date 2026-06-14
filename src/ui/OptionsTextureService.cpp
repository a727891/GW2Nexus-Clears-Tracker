#include "ui/OptionsTextureService.h"

#include "data/StaticDataLoader.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>
#include <unordered_map>

#include "nexus/Nexus.h"

namespace rc {
namespace OptionsTextureService {
namespace {

AddonAPI_t* g_api = nullptr;
std::string g_addonDir;
std::mutex g_mutex;
std::unordered_map<std::string, TextureInfo> g_textures;
std::atomic<bool> g_downloadRequested{false};
std::atomic<bool> g_downloadInFlight{false};

std::string TextureIdentifier(const std::string& filename) {
    return "NRC_OPT_" + filename;
}

TextureInfo TextureFromFile(const std::string& filename) {
    TextureInfo info;
    if (!g_api || !g_api->Textures_GetOrCreateFromFile) return info;

    const auto path =
        (std::filesystem::path(g_addonDir) / StaticDataLoader::kCacheDirName / filename).string();
    if (!std::filesystem::exists(path)) return info;

    Texture_t* texture = g_api->Textures_GetOrCreateFromFile(TextureIdentifier(filename).c_str(),
                                                             path.c_str());
    if (!texture || !texture->Resource) return info;

    info.texture = reinterpret_cast<ImTextureID>(texture->Resource);
    info.width = texture->Width;
    info.height = texture->Height;
    return info;
}

std::vector<std::string> LoadAssetManifest() {
    std::vector<std::string> assets;
    const auto manifestPath =
        (std::filesystem::path(g_addonDir) / StaticDataLoader::kCacheDirName /
         "clears_tracker.json")
            .string();
    if (!std::filesystem::exists(manifestPath)) return assets;

    try {
        std::ifstream in(manifestPath);
        nlohmann::json j;
        in >> j;
        if (j.contains("assets") && j["assets"].is_array()) {
            for (const auto& asset : j["assets"]) {
                if (asset.is_string()) {
                    assets.push_back(asset.get<std::string>());
                }
            }
        }
    } catch (...) {
    }
    return assets;
}

void DownloadAssetsWorker() {
    const auto assets = LoadAssetManifest();
    for (const auto& asset : assets) {
        if (std::filesystem::exists(
                std::filesystem::path(g_addonDir) / StaticDataLoader::kCacheDirName / asset)) {
            continue;
        }
        std::string content;
        StaticDataLoader::DownloadAsset(g_addonDir, asset, content);
    }
    g_downloadInFlight.store(false);
}

void EnsureDownloadRequested() {
    if (g_downloadRequested.exchange(true)) return;
    g_downloadInFlight.store(true);
    std::thread(DownloadAssetsWorker).detach();
}

void RefreshLoadedTextures() {
    const auto assets = LoadAssetManifest();
    std::lock_guard lock(g_mutex);
    for (const auto& asset : assets) {
        if (g_textures.count(asset) > 0 && g_textures[asset].texture != nullptr) continue;
        if (auto info = TextureFromFile(asset); info.texture) {
            g_textures[asset] = info;
        }
    }
}

TextureInfo LoadTextureInfo(const std::string& filename) {
    {
        std::lock_guard lock(g_mutex);
        const auto it = g_textures.find(filename);
        if (it != g_textures.end() && it->second.texture != nullptr) {
            return it->second;
        }
    }

    if (auto info = TextureFromFile(filename); info.texture) {
        std::lock_guard lock(g_mutex);
        g_textures[filename] = info;
        return info;
    }
    return {};
}

}  // namespace

void Initialize(AddonAPI_t* api, const std::string& addonDir) {
    g_api = api;
    g_addonDir = addonDir;
}

void RequestAssets() {
    EnsureDownloadRequested();
    RefreshLoadedTextures();
}

ImTextureID GetTexture(const std::string& filename) {
    return LoadTextureInfo(filename).texture;
}

TextureInfo GetTextureInfo(const std::string& filename) {
    return LoadTextureInfo(filename);
}

ImTextureID BackgroundTexture() { return GetTexture("texture_background.png"); }

}  // namespace OptionsTextureService
}  // namespace rc
