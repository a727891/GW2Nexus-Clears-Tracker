#include "data/FractalMapData.h"

#include <nlohmann/json.hpp>

namespace rc {

FractalMapData FractalMapData::FromJson(const nlohmann::json& j) {
    FractalMapData data;
    if (j.contains("DailyTier")) {
        for (const auto& day : j["DailyTier"]) {
            std::vector<std::string> names;
            for (const auto& name : day) {
                names.push_back(name.get<std::string>());
            }
            data.dailyTier.push_back(std::move(names));
        }
    }
    if (j.contains("Recs")) {
        for (const auto& day : j["Recs"]) {
            std::vector<int> scales;
            for (const auto& scale : day) {
                scales.push_back(scale.get<int>());
            }
            data.recs.push_back(std::move(scales));
        }
    }
    if (j.contains("challengeMotes")) {
        for (const auto& scale : j["challengeMotes"]) {
            data.challengeMotes.push_back(scale.get<int>());
        }
    }
    if (j.contains("scales")) {
        for (auto it = j["scales"].begin(); it != j["scales"].end(); ++it) {
            data.scales[it.key()] = it.value().get<std::string>();
        }
    }
    if (j.contains("maps")) {
        for (auto it = j["maps"].begin(); it != j["maps"].end(); ++it) {
            const auto& m = it.value();
            FractalMap map;
            map.label = m.value("label", "undefined");
            map.shortLabel = m.value("short", "undefined");
            map.apiLabel = m.value("api", "undefined");
            map.mapId = m.value("id", 0);
            if (m.contains("scales")) {
                for (const auto& scale : m["scales"]) {
                    map.scales.push_back(scale.get<int>());
                }
            }
            data.maps[it.key()] = std::move(map);
        }
    }
    return data;
}

FractalMap FractalMapData::GetFractalByName(const std::string& name) const {
    if (auto it = maps.find(name); it != maps.end()) {
        return it->second;
    }
    for (const auto& [_, map] : maps) {
        if (map.apiLabel == name || map.label == name) {
            return map;
        }
    }
    return {};
}

FractalMap FractalMapData::GetFractalForScale(int scale) const {
    const auto key = std::to_string(scale);
    if (auto scaleIt = scales.find(key); scaleIt != scales.end()) {
        return GetFractalByName(scaleIt->second);
    }
    return {};
}

const FractalMap* FractalMapData::GetFractalMapById(int mapId) const {
    for (const auto& [_, map] : maps) {
        if (map.mapId == mapId) {
            return &map;
        }
    }
    return nullptr;
}

}  // namespace rc
