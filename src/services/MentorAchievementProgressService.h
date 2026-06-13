#pragma once

#include "data/RaidData.h"
#include "services/Gw2ApiClient.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace rc {

struct MentorProgressEntry {
    int id = 0;
    int current = 0;
    int max = 0;
    bool done = false;
};

class MentorAchievementProgressService {
public:
    static constexpr int kDefaultMax = 1000;

    void Initialize(const RaidData& raidData, const std::string& cachePath);
    void LoadCache();
    void SaveCache() const;
    void RefreshFromApi(Gw2ApiClient& api, bool enabled);

    std::optional<MentorProgressEntry> GetProgress(int achievementId) const;
    int GetMaxForAchievement(int achievementId) const;

private:
    std::string cachePath_;
    std::unordered_map<int, int> mentorMaxById_;
    mutable std::mutex mutex_;
    std::unordered_map<int, MentorProgressEntry> progress_;
};

}  // namespace rc
