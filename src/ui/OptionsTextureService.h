#pragma once

#include "nexus/Nexus.h"

#include <imgui.h>
#include <string>

namespace rc {

namespace OptionsTextureService {

void Initialize(AddonAPI_t* api, const std::string& addonDir);
void RequestAssets();
ImTextureID GetTexture(const std::string& filename);
ImTextureID BackgroundTexture();

}  // namespace OptionsTextureService
}  // namespace rc
