#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace rc {

class InstabilitiesData {
public:
    std::vector<std::string> names;

    static InstabilitiesData FromJson(const nlohmann::json& j);
    static bool LoadOrDownload(const std::string& addonDir, InstabilitiesData& out);

    std::vector<std::string> GetInstabsForLevelOnDay(int level, int day) const;

private:
    std::unordered_map<std::string, std::vector<std::vector<int>>> instabilities_;
};

}  // namespace rc
