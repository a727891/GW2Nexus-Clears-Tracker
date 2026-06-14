#include "data/StrikeData.h"

namespace rc {

StrikeData StrikeData::FromJson(const nlohmann::json& j) {
    StrikeData data;
    data.version = j.value("version", "");
    data.weeklyAchievementId = j.value("weekly_achievement_id", 0);
    if (j.contains("priority")) {
        data.priority.id = j["priority"].value("id", "");
        data.priority.name = j["priority"].value("name", "Daily Raid Encounter Bounties");
        data.priority.abbreviation = j["priority"].value("abbriviation", "DRB");
    }
    if (j.contains("priority_tomorrow")) {
        data.priorityTomorrow.id = j["priority_tomorrow"].value("id", "");
        data.priorityTomorrow.name =
            j["priority_tomorrow"].value("name", "Tomorrow's Raid Encounter Bounties");
        data.priorityTomorrow.abbreviation = j["priority_tomorrow"].value("abbriviation", "DRB+");
    }
    if (j.contains("weekly_achievement_bit_strike_ids")) {
        for (const auto& id : j["weekly_achievement_bit_strike_ids"]) {
            data.weeklyAchievementBitStrikeIds.push_back(id.get<std::string>());
        }
    }
    if (j.contains("map_tracked_strike_ids")) {
        for (const auto& id : j["map_tracked_strike_ids"]) {
            data.mapTrackedStrikeIds.push_back(id.get<std::string>());
        }
    }
    if (!j.contains("expansions")) return data;

    for (const auto& expJ : j["expansions"]) {
        ExpansionStrikes exp;
        exp.id = expJ.value("id", "");
        exp.name = expJ.value("name", "");
        exp.abbreviation = expJ.value("abbriviation", "");
        exp.resets = expJ.value("resets", "weekly");
        if (expJ.contains("missions")) {
            for (const auto& mJ : expJ["missions"]) {
                StrikeMission m;
                m.id = mJ.value("id", "");
                m.name = mJ.value("name", "");
                m.abbreviation = mJ.value("abbriviation", "");
                m.assetId = mJ.value("assetId", 0);
                m.powerFavored = mJ.value("powerFavored", false);
                m.condiFavored = mJ.value("condiFavored", false);
                m.needsDefianceBreak = mJ.value("needsDefianceBreak", false);
                m.resets = mJ.value("resets", "");
                if (mJ.contains("mapIds")) {
                    for (const auto& mapId : mJ["mapIds"]) {
                        m.mapIds.push_back(mapId.get<int>());
                    }
                }
                if (mJ.contains("daily_bounty_achievement_id") &&
                    !mJ["daily_bounty_achievement_id"].is_null()) {
                    m.dailyBountyAchievementId =
                        mJ["daily_bounty_achievement_id"].get<int>();
                }
                exp.missions.push_back(std::move(m));
            }
        }
        data.expansions.push_back(std::move(exp));
    }
    return data;
}

const StrikeMission* StrikeData::GetMissionById(const std::string& id) const {
    for (const auto& exp : expansions) {
        for (const auto& m : exp.missions) {
            if (m.id == id) return &m;
        }
    }
    return nullptr;
}

const StrikeMission* StrikeData::GetMissionByMapId(uint32_t mapId) const {
    for (const auto& exp : expansions) {
        for (const auto& m : exp.missions) {
            for (int mid : m.mapIds) {
                if (static_cast<uint32_t>(mid) == mapId) return &m;
            }
        }
    }
    return nullptr;
}

bool StrikeData::IsMapTracked(const std::string& id) const {
    for (const auto& tracked : mapTrackedStrikeIds) {
        if (tracked == id) return true;
    }
    return false;
}

std::string StrikeData::GetMissionResetById(const std::string& id) const {
    for (const auto& exp : expansions) {
        for (const auto& m : exp.missions) {
            if (m.id == id) {
                return !m.resets.empty() ? m.resets : exp.resets;
            }
        }
    }
    return "weekly";
}

}  // namespace rc
