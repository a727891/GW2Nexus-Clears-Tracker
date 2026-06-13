#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace rc {

struct BossEncounter {
    std::string apiId;
    std::string name;
    std::string abbreviation;
    int assetId = 0;
    std::vector<int> mapIds;
    std::optional<int> dailyBountyAchievementId;
    std::optional<int> mentorAchievementId;
    std::optional<int> mentorAchievementMax;
    std::string resets;
    bool powerFavored = false;
    bool condiFavored = false;
    bool needsDefianceBreak = false;

    std::string EncounterId() const {
        return !apiId.empty() && apiId != "undefined" ? apiId : name;
    }

    static BossEncounter FromJson(const nlohmann::json& j, bool isStrike);
};

struct RaidWing {
    std::string id;
    std::string name;
    std::string abbreviation;
    int number = 0;
    int callOfTheMistsTimestamp = 0;
    int callOfTheMistsWeeks = 0;
    int emboldenedTimestamp = 0;
    int emboldenedWeeks = 0;
    std::vector<BossEncounter> encounters;
};

struct ExpansionRaid {
    std::string id;
    std::string name;
    std::vector<RaidWing> wings;
};

class RaidData {
public:
    std::string version;
    int secondsInWeek = 604800;
    int powerDamageAssetId = 0;
    int condiDamageAssetId = 0;
    int defianceAssetId = 0;
    int mentorAssetId = 0;
    std::vector<std::string> eventEncounterApiIds;
    std::vector<ExpansionRaid> expansions;

    static RaidData FromJson(const nlohmann::json& j);
    const BossEncounter* GetEncounterByApiId(const std::string& apiId) const;
    const BossEncounter* GetEncounterById(const std::string& id) const;
    const RaidWing* GetWingById(const std::string& wingId) const;
    bool IsEventEncounter(const std::string& apiId) const;
};

}  // namespace rc
