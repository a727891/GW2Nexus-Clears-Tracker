#pragma once

#include "data/RaidData.h"
#include "services/Gw2ApiClient.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace rc {

struct MentorProgressEntry {
    int id = 0;
    int current = 0;
    int max = 0;
    bool done = false;
};

struct MentorProgressChange {
    int achievementId = 0;
    int previousCurrent = 0;
    int newCurrent = 0;

    int Delta() const { return newCurrent - previousCurrent; }
};

class MentorAchievementProgressService {
public:
    static constexpr int kDefaultMax = 1000;

    void Initialize(const RaidData& raidData, const std::string& cachePath);
    void SetActiveAccount(const std::string& accountName);
    void LoadCache();
    void SaveCache() const;
    std::vector<MentorProgressChange> RefreshFromApi(Gw2ApiClient& api, bool enabled);

    std::optional<MentorProgressEntry> GetProgress(int achievementId) const;
    int GetMaxForAchievement(int achievementId) const;

private:
    void LoadAccountSlice(const std::string& accountName);
    void SaveAccountSlice(const std::string& accountName) const;

    std::string cachePath_;
    std::string activeAccount_;
    std::unordered_map<int, int> mentorMaxById_;
    mutable std::mutex mutex_;
    std::unordered_map<int, MentorProgressEntry> progress_;
    std::unordered_map<std::string, std::unordered_map<int, MentorProgressEntry>> accountProgress_;
};

}  // namespace rc
