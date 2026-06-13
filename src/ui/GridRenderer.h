#pragma once

#include "core/SettingsStore.h"
#include "core/Types.h"
#include "data/RaidData.h"
#include "data/StrikeData.h"
#include "services/MentorAchievementProgressService.h"

#include <imgui.h>
#include <vector>

struct ImFont;

namespace rc {

struct GridDrawContext {
    const RaidData* raidData = nullptr;
    const StrikeData* strikeData = nullptr;
    const MentorAchievementProgressService* mentorProgress = nullptr;
    bool isStrikePanel = false;
};

namespace GridRenderer {
ImVec2 DrawGroups(const std::vector<GridGroup>& groups,
                  const SettingsStore& settings,
                  bool colorClears,
                  bool useNonWeeklyHighlight = false,
                  ImFont* font = nullptr,
                  const GridDrawContext& context = {},
                  bool useDungeonFrequenter = false);
}

}  // namespace rc
