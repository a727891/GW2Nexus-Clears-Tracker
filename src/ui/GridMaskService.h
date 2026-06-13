#pragma once

#include "nexus/Nexus.h"

#include <imgui.h>
#include <cstdint>
#include <string>

namespace rc {
namespace GridMaskService {

void Initialize(AddonAPI_t* api, const std::string& addonDir);
bool HasMasks();

struct OrganicCellStyle {
    ImTextureID texture = nullptr;
    float xOffset = 0.0f;
    float yOffset = 0.0f;
    float widthDelta = 0.0f;
    float heightDelta = 0.0f;
};

OrganicCellStyle StyleForSeed(uint32_t seed);
uint32_t HashSeed(const std::string& groupId, const std::string& cellId = {});

}  // namespace GridMaskService
}  // namespace rc
