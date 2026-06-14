#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace rc {

struct BossSlotRotation {
    int slot = 0;
    int offset = 0;
    std::vector<std::string> encounters;
};

class DailyBountyData {
public:
    bool enabled = false;
    std::string version;
    std::string name = "Daily Raid Encounter Bounties";
    std::string abbreviation = "DRB";
    std::string tomorrowName = "Tomorrow's Raid Encounter Bounties";
    std::string tomorrowAbbreviation = "DRB+";
    std::string dailyBountyCategoryUrl =
        "https://api.guildwars2.com/v2/achievements/categories/475";
    std::vector<BossSlotRotation> bossSlots;

    static DailyBountyData FromJson(const nlohmann::json& j);
};

}  // namespace rc
