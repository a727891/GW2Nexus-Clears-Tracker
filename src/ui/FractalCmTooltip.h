#pragma once

#include "data/FractalMapData.h"
#include "data/InstabilitiesData.h"

#include <imgui.h>
#include <string>

namespace rc {
namespace FractalCmTooltip {

void ShowIfHovered(const ImVec2& p0,
                   const ImVec2& p1,
                   const FractalMap& map,
                   const std::string& customAbbrev,
                   int scale,
                   const FractalMapData& fractalMapData,
                   const InstabilitiesData& instabilitiesData);

}  // namespace FractalCmTooltip
}  // namespace rc
