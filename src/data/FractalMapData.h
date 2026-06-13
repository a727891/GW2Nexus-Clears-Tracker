#pragma once

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>

namespace rc {

struct FractalMap {
    std::string label;
    std::string shortLabel;
    std::string apiLabel;
    std::vector<int> scales;
    int mapId = 0;

    bool IsValid() const { return !apiLabel.empty() && apiLabel != "undefined"; }
};

class FractalMapData {
public:
    std::vector<std::vector<std::string>> dailyTier;
    std::vector<std::vector<int>> recs;
    std::vector<int> challengeMotes;
    std::map<std::string, FractalMap> maps;
    std::map<std::string, std::string> scales;

    static FractalMapData FromJson(const nlohmann::json& j);

    FractalMap GetFractalByName(const std::string& name) const;
    FractalMap GetFractalForScale(int scale) const;
    const FractalMap* GetFractalMapById(int mapId) const;
};

}  // namespace rc
