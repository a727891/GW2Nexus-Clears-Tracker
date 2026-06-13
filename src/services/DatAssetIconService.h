#pragma once

#include "nexus/Nexus.h"

#include <imgui.h>
#include <mutex>
#include <string>
#include <unordered_set>

namespace rc {

class DatAssetIconService {
public:
    static void Initialize(AddonAPI_t* api, const std::string& addonDir);
    static void Shutdown();
    static void PreloadIndicators(int powerId, int condiId, int defianceId, int mentorId);
    static ImTextureID Request(int assetId);

private:
    static ImTextureID LoadFromDisk(int assetId);
    static void QueueDownload(int assetId);
    static void ProcessDownloads();
};

}  // namespace rc
