#pragma once

#include "data/RaidData.h"
#include "data/StrikeData.h"
#include "services/MentorAchievementProgressService.h"

#include <imgui.h>
#include <optional>
#include <string>

namespace rc {

struct EncounterTooltipData {
    std::string name;
    std::string abbreviation;
    int assetId = 0;
    bool powerFavored = false;
    bool condiFavored = false;
    bool needsDefianceBreak = false;
    std::optional<int> mentorAchievementId;
    std::optional<int> mentorAchievementMax;
    bool isStrike = false;
};

namespace EncounterTooltip {

std::optional<EncounterTooltipData> BuildFromRaid(const RaidData& raidData,
                                                  const std::string& encounterId);
std::optional<EncounterTooltipData> BuildFromStrike(const StrikeData& strikeData,
                                                     const std::string& encounterId);

void ShowIfHovered(const ImVec2& p0,
                   const ImVec2& p1,
                   const EncounterTooltipData& data,
                   const RaidData& raidData,
                   const MentorAchievementProgressService* mentorProgress,
                   bool showMentorProgress);

}  // namespace EncounterTooltip
}  // namespace rc
