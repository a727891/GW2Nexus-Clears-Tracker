#pragma once

#include "core/SettingsStore.h"
#include "core/Types.h"
#include "data/RaidData.h"

#include <imgui.h>
#include <vector>

struct ImFont;

namespace rc {

namespace GridRenderer {
    ImVec2 DrawGroups(const std::vector<GridGroup>& groups,
                      const SettingsStore& settings,
                      bool colorClears,
                      bool useNonWeeklyHighlight = false,
                      ImFont* font = nullptr,
                      const RaidData* raidData = nullptr);
}

}  // namespace rc
