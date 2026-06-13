#include "services/Gw2ApiClient.h"

#include "services/DungeonsClearsService.h"
#include "services/HttpClient.h"

#include <nlohmann/json.hpp>

namespace rc {

void Gw2ApiClient::SetApiKey(const std::string& key) {
    std::lock_guard lock(mutex_);
    apiKey_ = key;
}

std::optional<std::string> Gw2ApiClient::HttpGet(const std::string& path, bool auth) {
    std::string token;
    {
        std::lock_guard lock(mutex_);
        if (auth && apiKey_.empty()) return std::nullopt;
        token = apiKey_;
    }

    HttpRequestOptions options;
    if (auth) options.bearerToken = token;
    return HttpGetUrl("https://api.guildwars2.com" + path, options);
}

std::optional<std::unordered_set<std::string>> Gw2ApiClient::FetchRaidClears() {
    auto body = HttpGet("/v2/account/raids", true);
    if (!body) return std::nullopt;

    try {
        auto j = nlohmann::json::parse(*body);
        std::unordered_set<std::string> clears;
        for (const auto& id : j) {
            clears.insert(id.get<std::string>());
        }
        return clears;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::unordered_set<std::string>> Gw2ApiClient::FetchDungeonClears() {
    auto body = HttpGet("/v2/account/dungeons", true);
    if (!body) return std::nullopt;

    try {
        auto j = nlohmann::json::parse(*body);
        std::unordered_set<std::string> clears;
        for (const auto& id : j) {
            clears.insert(id.get<std::string>());
        }
        return clears;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::unordered_set<std::string>> Gw2ApiClient::FetchFrequenterPaths() {
    auto achievements = FetchAccountAchievements();
    if (!achievements) return std::nullopt;

    std::unordered_set<std::string> paths;
    for (const auto& ach : *achievements) {
        if (ach.id != 2963) continue;
        for (const int bit : ach.bits) {
            const auto path = DungeonsClearsService::FrequenterBitToPathId(bit);
            if (!path.empty()) {
                paths.insert(path);
            }
        }
        break;
    }
    return paths;
}

std::optional<std::vector<AccountAchievement>> Gw2ApiClient::FetchAccountAchievements() {
    auto body = HttpGet("/v2/account/achievements", true);
    if (!body) return std::nullopt;

    try {
        auto j = nlohmann::json::parse(*body);
        std::vector<AccountAchievement> achievements;
        for (const auto& ach : j) {
            AccountAchievement a;
            a.id = ach.value("id", 0);
            a.current = ach.value("current", 0);
            a.max = ach.value("max", 0);
            a.done = ach.value("done", false);
            if (ach.contains("bits")) {
                for (const auto& bit : ach["bits"]) {
                    a.bits.push_back(bit.get<int>());
                }
            }
            achievements.push_back(std::move(a));
        }
        return achievements;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::vector<int>> Gw2ApiClient::FetchAchievementCategoryIds(const std::string& url) {
    HttpRequestOptions options;
    const auto body = HttpGetUrl(url, options);
    if (!body) return std::nullopt;

    try {
        auto j = nlohmann::json::parse(*body);
        std::vector<int> ids;
        if (j.contains("achievements")) {
            for (const auto& id : j["achievements"]) {
                ids.push_back(id.get<int>());
            }
        }
        return ids;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> Gw2ApiClient::FetchAccountName() {
    auto body = HttpGet("/v2/account", true);
    if (!body) return std::nullopt;
    try {
        auto j = nlohmann::json::parse(*body);
        return j.value("name", "");
    } catch (...) {
        return std::nullopt;
    }
}

TokenInfo Gw2ApiClient::ValidateToken() {
    TokenInfo info;
    auto body = HttpGet("/v2/tokeninfo", true);
    if (!body) return info;

    try {
        auto j = nlohmann::json::parse(*body);
        info.valid = true;
        if (j.contains("permissions")) {
            for (const auto& p : j["permissions"]) {
                info.permissions.push_back(p.get<std::string>());
            }
        }
    } catch (...) {
        info.valid = false;
    }
    return info;
}

}  // namespace rc
