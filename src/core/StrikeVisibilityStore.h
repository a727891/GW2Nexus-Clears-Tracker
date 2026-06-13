#pragma once

#include "data/StrikeData.h"

#include <string>
#include <unordered_map>

namespace rc {

class StrikeVisibilityStore {
public:
    std::string version = "1.0.0";
    bool priorityVisible = true;
    bool tomorrowBountiesVisible = false;
    std::unordered_map<std::string, bool> expansions;
    std::unordered_map<std::string, bool> missions;

    void Load(const std::string& path);
    void Save(const std::string& path) const;
    void InitializeFromData(const StrikeData& data);

    bool IsExpansionVisible(const std::string& expansionId) const;
    bool IsMissionVisible(const std::string& missionId) const;
    bool IsPriorityVisible() const { return priorityVisible; }
    bool IsTomorrowBountiesVisible() const { return tomorrowBountiesVisible; }

    void SetExpansionVisible(const std::string& expansionId, bool visible);
    void SetMissionVisible(const std::string& missionId, bool visible);
    void SetPriorityVisible(bool visible) { priorityVisible = visible; }
    void SetTomorrowBountiesVisible(bool visible) { tomorrowBountiesVisible = visible; }
};

}  // namespace rc
