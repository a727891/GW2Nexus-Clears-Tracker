#include "data/RaidData.h"

#include <cctype>

namespace rc {

namespace {

constexpr const char* kDefaultEventEncounterApiIds[] = {
    "spirit_woods", "bandit_trio", "escort",      "twisted_castle",
    "river_of_souls", "statues_of_grenth", "gate", "camp",
};

bool EqualsIgnoreCase(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

bool IsInEventEncounterList(const std::string& apiId,
                            const std::vector<std::string>& eventIds) {
    for (const auto& id : eventIds) {
        if (EqualsIgnoreCase(id, apiId)) return true;
    }
    return false;
}

}  // namespace

BossEncounter BossEncounter::FromJson(const nlohmann::json& j, bool isStrike) {
    BossEncounter e;
    if (isStrike) {
        e.apiId = j.value("id", "");
        e.name = j.value("name", "");
        e.abbreviation = j.value("abbriviation", "");
    } else {
        e.apiId = j.value("api_id", "");
        e.name = j.value("name", "");
        e.abbreviation = j.value("abbriviation", "");
    }
    e.assetId = j.value("assetId", 0);
    if (j.contains("mapIds")) {
        for (const auto& id : j["mapIds"]) {
            e.mapIds.push_back(id.get<int>());
        }
    }
    if (j.contains("daily_bounty_achievement_id") && !j["daily_bounty_achievement_id"].is_null()) {
        e.dailyBountyAchievementId = j["daily_bounty_achievement_id"].get<int>();
    }
    e.resets = j.value("resets", "");
    return e;
}

RaidData RaidData::FromJson(const nlohmann::json& j) {
    RaidData data;
    data.version = j.value("version", "");
    if (j.contains("eventEncounterApiIds")) {
        for (const auto& id : j["eventEncounterApiIds"]) {
            data.eventEncounterApiIds.push_back(id.get<std::string>());
        }
    }
    if (data.eventEncounterApiIds.empty()) {
        for (const auto* id : kDefaultEventEncounterApiIds) {
            data.eventEncounterApiIds.emplace_back(id);
        }
    }
    if (!j.contains("expansions")) return data;

    for (const auto& expJ : j["expansions"]) {
        ExpansionRaid exp;
        exp.id = expJ.value("id", "");
        exp.name = expJ.value("name", "");
        if (!expJ.contains("wings")) continue;
        for (const auto& wingJ : expJ["wings"]) {
            RaidWing wing;
            wing.id = wingJ.value("id", "");
            wing.name = wingJ.value("name", "");
            wing.abbreviation = wingJ.value("abbriviation", "");
            wing.number = wingJ.value("number", 0);
            if (wingJ.contains("encounters")) {
                for (const auto& encJ : wingJ["encounters"]) {
                    wing.encounters.push_back(BossEncounter::FromJson(encJ, false));
                }
            }
            exp.wings.push_back(std::move(wing));
        }
        data.expansions.push_back(std::move(exp));
    }
    return data;
}

const BossEncounter* RaidData::GetEncounterByApiId(const std::string& apiId) const {
    for (const auto& exp : expansions) {
        for (const auto& wing : exp.wings) {
            for (const auto& enc : wing.encounters) {
                if (enc.EncounterId() == apiId) return &enc;
            }
        }
    }
    return nullptr;
}

bool RaidData::IsEventEncounter(const std::string& apiId) const {
    return IsInEventEncounterList(apiId, eventEncounterApiIds);
}

}  // namespace rc
