#include "core/StrikeVisibilityStore.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace rc {
namespace {

constexpr const char* kCurrentVersion = "1.0.0";

bool MapValue(const std::unordered_map<std::string, bool>& values, const std::string& key) {
    const auto it = values.find(key);
    return it == values.end() ? true : it->second;
}

void LoadBoolMap(const nlohmann::json& j, const char* key,
                 std::unordered_map<std::string, bool>& out) {
    if (!j.contains(key) || !j[key].is_object()) return;
    for (const auto& [id, visible] : j[key].items()) {
        if (visible.is_boolean()) {
            out[id] = visible.get<bool>();
        }
    }
}

nlohmann::json BoolMapToJson(const std::unordered_map<std::string, bool>& values) {
    nlohmann::json j = nlohmann::json::object();
    for (const auto& [id, visible] : values) {
        j[id] = visible;
    }
    return j;
}

}  // namespace

void StrikeVisibilityStore::Load(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return;

    nlohmann::json j;
    in >> j;
    version = j.value("version", kCurrentVersion);
    priorityVisible = j.value("priority", true);
    tomorrowBountiesVisible = j.value("tomorrow_bounties", false);
    LoadBoolMap(j, "expansions", expansions);
    LoadBoolMap(j, "missions", missions);
}

void StrikeVisibilityStore::Save(const std::string& path) const {
    nlohmann::json j = {{"version", version},
                        {"priority", priorityVisible},
                        {"tomorrow_bounties", tomorrowBountiesVisible},
                        {"expansions", BoolMapToJson(expansions)},
                        {"missions", BoolMapToJson(missions)}};

    std::ofstream out(path);
    out << j.dump(2);
}

void StrikeVisibilityStore::InitializeFromData(const StrikeData& data) {
    for (const auto& exp : data.expansions) {
        expansions.try_emplace(exp.id, true);
        for (const auto& mission : exp.missions) {
            missions.try_emplace(mission.id, true);
        }
    }
}

bool StrikeVisibilityStore::IsExpansionVisible(const std::string& expansionId) const {
    return MapValue(expansions, expansionId);
}

bool StrikeVisibilityStore::IsMissionVisible(const std::string& missionId) const {
    return MapValue(missions, missionId);
}

void StrikeVisibilityStore::SetExpansionVisible(const std::string& expansionId, bool visible) {
    expansions[expansionId] = visible;
}

void StrikeVisibilityStore::SetMissionVisible(const std::string& missionId, bool visible) {
    missions[missionId] = visible;
}

}  // namespace rc
