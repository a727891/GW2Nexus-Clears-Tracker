#include "core/StrikeVisibilityStore.h"
#include "core/StorageKeyUtil.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

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

nlohmann::json StringMapToJson(const std::unordered_map<std::string, std::string>& values) {
    nlohmann::json j = nlohmann::json::object();
    for (const auto& [id, label] : values) {
        j[id] = label;
    }
    return j;
}

void LoadStringMap(const nlohmann::json& j, const char* key,
                   std::unordered_map<std::string, std::string>& out) {
    if (!j.contains(key) || !j[key].is_object()) return;
    for (const auto& [id, label] : j[key].items()) {
        if (label.is_string()) {
            out[id] = label.get<std::string>();
        }
    }
}

}  // namespace

void StrikeVisibilityStore::MigratePriorityLabelKeys() {
    std::vector<std::string> keys;
    keys.reserve(encounterLabels.size());
    for (const auto& [key, _] : encounterLabels) {
        keys.push_back(key);
    }

    for (const auto& key : keys) {
        if (key == "priority" || key == "priority_tomorrow") continue;
        if (key.rfind(kPriorityPrefix, 0) != 0 && key.rfind(kTomorrowPrefix, 0) != 0) continue;

        const std::string baseKey =
            key.rfind(kPriorityPrefix, 0) == 0
                ? key.substr(std::strlen(kPriorityPrefix))
                : key.substr(std::strlen(kTomorrowPrefix));
        if (!encounterLabels.contains(baseKey)) {
            encounterLabels[baseKey] = encounterLabels[key];
        }
        encounterLabels.erase(key);
    }
}

std::string StrikeVisibilityStore::GetEncounterLabel(const std::string& encounterId,
                                                     const std::string& defaultAbbrev) const {
    if (const auto it = encounterLabels.find(encounterId); it != encounterLabels.end()) {
        return it->second;
    }
    const auto storageKey = NormalizeStorageKey(encounterId);
    if (storageKey != encounterId) {
        if (const auto it = encounterLabels.find(storageKey); it != encounterLabels.end()) {
            return it->second;
        }
    }
    return defaultAbbrev;
}

void StrikeVisibilityStore::SetEncounterLabel(const std::string& encounterId,
                                              const std::string& label) {
    const auto storageKey = NormalizeStorageKey(encounterId);
    encounterLabels[storageKey] = label;
}

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
    LoadStringMap(j, "encounterLabels", encounterLabels);
    MigratePriorityLabelKeys();
}

void StrikeVisibilityStore::Save(const std::string& path) const {
    nlohmann::json j = {{"version", version},
                        {"priority", priorityVisible},
                        {"tomorrow_bounties", tomorrowBountiesVisible},
                        {"expansions", BoolMapToJson(expansions)},
                        {"missions", BoolMapToJson(missions)},
                        {"encounterLabels", StringMapToJson(encounterLabels)}};

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
