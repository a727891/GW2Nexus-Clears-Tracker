#pragma once

#include "nexus/Nexus.h"

#include <imgui.h>
#include <cstdint>
#include <string>

namespace rc {

namespace ContentLogoService {

struct LogoInfo {
    ImTextureID texture = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

void Initialize(AddonAPI_t* api, const std::string& addonDir);
LogoInfo GetLogo(const char* expansionId);
bool RenderLogo(const char* expansionId);

}  // namespace ContentLogoService
}  // namespace rc
