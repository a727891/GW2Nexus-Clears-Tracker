#pragma once

#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace rc {

struct AccountAchievement {
    int id = 0;
    bool done = false;
    std::vector<int> bits;
};

struct TokenInfo {
    bool valid = false;
    std::vector<std::string> permissions;
};

class Gw2ApiClient {
public:
    void SetApiKey(const std::string& key);

    std::optional<std::unordered_set<std::string>> FetchRaidClears();
    std::optional<std::vector<AccountAchievement>> FetchAccountAchievements();
    std::optional<std::vector<int>> FetchAchievementCategoryIds(const std::string& url);
    std::optional<std::string> FetchAccountName();
    TokenInfo ValidateToken();

private:
    std::string apiKey_;
    std::mutex mutex_;

    std::optional<std::string> HttpGet(const std::string& path, bool auth);
};

}  // namespace rc
