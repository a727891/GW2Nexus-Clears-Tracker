#pragma once

#include "data/RaidData.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace rc {

struct StrikeMission {
    std::string id;
    std::string name;
    std::string abbreviation;
    std::vector<int> mapIds;
    std::string resets;
    std::optional<int> dailyBountyAchievementId;
};

struct ExpansionStrikes {
    std::string id;
    std::string name;
    std::string abbreviation;
    std::string resets = "weekly";
    std::vector<StrikeMission> missions;
};

struct PriorityMeta {
    std::string id;
    std::string name;
    std::string abbreviation;
};

class StrikeData {
public:
    std::string version;
    int weeklyAchievementId = 0;
    std::vector<std::string> weeklyAchievementBitStrikeIds;
    std::vector<std::string> mapTrackedStrikeIds;
    PriorityMeta priority;
    PriorityMeta priorityTomorrow;
    std::vector<ExpansionStrikes> expansions;

    static StrikeData FromJson(const nlohmann::json& j);
    const StrikeMission* GetMissionById(const std::string& id) const;
    const StrikeMission* GetMissionByMapId(uint32_t mapId) const;
    bool IsMapTracked(const std::string& id) const;
    std::string GetMissionResetById(const std::string& id) const;
};

}  // namespace rc
