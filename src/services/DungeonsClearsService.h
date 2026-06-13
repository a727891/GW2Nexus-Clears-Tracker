#pragma once

#include "services/Gw2ApiClient.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace rc {

class DungeonsClearsService {
public:
    static constexpr int kFrequenterAchievementId = 2963;

    static std::unordered_set<std::string> FetchWeeklyClears(Gw2ApiClient& api);
    static std::unordered_set<std::string> FetchFrequenterPaths(Gw2ApiClient& api);
    static std::string FrequenterBitToPathId(int bit);
};

}  // namespace rc
