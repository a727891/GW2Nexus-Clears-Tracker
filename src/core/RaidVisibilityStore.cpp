#include "core/RaidVisibilityStore.h"
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

void RaidVisibilityStore::MigratePriorityLabelKeys() {
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

std::string RaidVisibilityStore::GetEncounterLabel(const std::string& encounterId,
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

void RaidVisibilityStore::SetEncounterLabel(const std::string& encounterId,
                                            const std::string& label) {
    const auto storageKey = NormalizeStorageKey(encounterId);
    encounterLabels[storageKey] = label;
}

void RaidVisibilityStore::Load(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return;

    nlohmann::json j;
    in >> j;
    version = j.value("version", kCurrentVersion);
    LoadBoolMap(j, "expansions", expansions);
    LoadBoolMap(j, "wings", wings);
    LoadBoolMap(j, "encounters", encounters);
    LoadStringMap(j, "encounterLabels", encounterLabels);
    MigratePriorityLabelKeys();
}

void RaidVisibilityStore::Save(const std::string& path) const {
    nlohmann::json j = {{"version", version},
                        {"expansions", BoolMapToJson(expansions)},
                        {"wings", BoolMapToJson(wings)},
                        {"encounters", BoolMapToJson(encounters)},
                        {"encounterLabels", StringMapToJson(encounterLabels)}};

    std::ofstream out(path);
    out << j.dump(2);
}

void RaidVisibilityStore::InitializeFromData(const RaidData& data) {
    for (const auto& exp : data.expansions) {
        expansions.try_emplace(exp.id, true);
        for (const auto& wing : exp.wings) {
            wings.try_emplace(wing.id, true);
            for (const auto& enc : wing.encounters) {
                encounters.try_emplace(enc.EncounterId(), true);
            }
        }
    }
}

bool RaidVisibilityStore::IsExpansionVisible(const std::string& expansionId) const {
    return MapValue(expansions, expansionId);
}

bool RaidVisibilityStore::IsWingVisible(const std::string& wingId) const {
    return MapValue(wings, wingId);
}

bool RaidVisibilityStore::IsEncounterVisible(const std::string& encounterId) const {
    return MapValue(encounters, encounterId);
}

void RaidVisibilityStore::SetExpansionVisible(const std::string& expansionId, bool visible,
                                              const RaidData& data) {
    expansions[expansionId] = visible;
    for (const auto& exp : data.expansions) {
        if (exp.id != expansionId) continue;
        for (const auto& wing : exp.wings) {
            wings[wing.id] = visible;
        }
        break;
    }
}

void RaidVisibilityStore::SetWingVisible(const std::string& wingId, bool visible) {
    wings[wingId] = visible;
}

void RaidVisibilityStore::SetEncounterVisible(const std::string& encounterId, bool visible) {
    encounters[encounterId] = visible;
}

}  // namespace rc
