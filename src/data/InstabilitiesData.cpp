#include "data/InstabilitiesData.h"

#include "data/StaticDataLoader.h"

#include <nlohmann/json.hpp>

namespace rc {

InstabilitiesData InstabilitiesData::FromJson(const nlohmann::json& j) {
    InstabilitiesData data;
    if (j.contains("instability_names") && j["instability_names"].is_array()) {
        for (const auto& name : j["instability_names"]) {
            if (name.is_string()) {
                data.names.push_back(name.get<std::string>());
            }
        }
    }

    if (!j.contains("instabilities") || !j["instabilities"].is_object()) {
        return data;
    }

    for (const auto& [levelKey, days] : j["instabilities"].items()) {
        if (!days.is_array()) continue;

        std::vector<std::vector<int>> dayRows;
        dayRows.resize(days.size());
        for (size_t day = 0; day < days.size(); ++day) {
            if (!days[day].is_array()) continue;
            for (const auto& idx : days[day]) {
                if (idx.is_number_integer()) {
                    dayRows[day].push_back(idx.get<int>());
                }
            }
        }
        data.instabilities_[levelKey] = std::move(dayRows);
    }

    return data;
}

bool InstabilitiesData::LoadFromCache(const std::string& addonDir, InstabilitiesData& out) {
    std::string json;
    if (!StaticDataLoader::LoadCached(addonDir, "fractal_instabilities.json", json)) {
        return false;
    }

    try {
        out = FromJson(nlohmann::json::parse(json));
        return true;
    } catch (...) {
        return false;
    }
}

bool InstabilitiesData::LoadOrDownload(const std::string& addonDir, InstabilitiesData& out) {
    std::string json;
    if (!StaticDataLoader::LoadOrDownload(addonDir, "fractal_instabilities.json", json)) {
        return false;
    }

    try {
        out = FromJson(nlohmann::json::parse(json));
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> InstabilitiesData::GetInstabsForLevelOnDay(int level, int day) const {
    std::vector<std::string> result;
    const auto it = instabilities_.find(std::to_string(level));
    if (it == instabilities_.end()) return result;
    if (day < 0 || day >= static_cast<int>(it->second.size())) return result;

    for (const int idx : it->second[static_cast<size_t>(day)]) {
        if (idx >= 0 && idx < static_cast<int>(names.size())) {
            result.push_back(names[static_cast<size_t>(idx)]);
        }
    }
    return result;
}

}  // namespace rc
