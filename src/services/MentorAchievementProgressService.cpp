#include "services/MentorAchievementProgressService.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace rc {
namespace {

nlohmann::json ProgressToJson(const std::unordered_map<int, MentorProgressEntry>& progress) {
    nlohmann::json achievements = nlohmann::json::array();
    for (const auto& [id, entry] : progress) {
        (void)id;
        achievements.push_back({{"id", entry.id},
                                {"current", entry.current},
                                {"max", entry.max},
                                {"done", entry.done}});
    }
    return achievements;
}

std::unordered_map<int, MentorProgressEntry> ProgressFromJson(const nlohmann::json& achievements,
                                                              const std::unordered_map<int, int>& mentorMaxById) {
    std::unordered_map<int, MentorProgressEntry> loaded;
    if (!achievements.is_array()) return loaded;

    for (const auto& entryJ : achievements) {
        MentorProgressEntry entry;
        entry.id = entryJ.value("id", 0);
        entry.current = entryJ.value("current", 0);
        entry.max = entryJ.value("max", MentorAchievementProgressService::kDefaultMax);
        entry.done = entryJ.value("done", false);
        if (mentorMaxById.count(entry.id) == 0) continue;
        entry.max = mentorMaxById.at(entry.id);
        loaded[entry.id] = entry;
    }
    return loaded;
}

}  // namespace

void MentorAchievementProgressService::Initialize(const RaidData& raidData,
                                                  const std::string& cachePath) {
    cachePath_ = cachePath;
    mentorMaxById_.clear();
    activeAccount_.clear();
    {
        std::lock_guard lock(mutex_);
        progress_.clear();
        accountProgress_.clear();
    }

    for (const auto& exp : raidData.expansions) {
        for (const auto& wing : exp.wings) {
            for (const auto& enc : wing.encounters) {
                if (!enc.mentorAchievementId) continue;
                const int id = *enc.mentorAchievementId;
                int max = enc.mentorAchievementMax.value_or(kDefaultMax);
                if (max <= 0) max = kDefaultMax;
                mentorMaxById_[id] = max;
            }
        }
    }
}

void MentorAchievementProgressService::SetActiveAccount(const std::string& accountName) {
    std::lock_guard lock(mutex_);
    if (!activeAccount_.empty()) {
        accountProgress_[activeAccount_] = progress_;
    }

    activeAccount_ = accountName;
    progress_.clear();

    if (accountName.empty()) return;

    if (const auto it = accountProgress_.find(accountName); it != accountProgress_.end()) {
        progress_ = it->second;
        return;
    }

    // Migrate legacy single-account cache (Blish / early Nexus format keyed as "").
    if (const auto legacy = accountProgress_.find(""); legacy != accountProgress_.end()) {
        progress_ = legacy->second;
        accountProgress_[accountName] = progress_;
    }
}

void MentorAchievementProgressService::LoadCache() {
    if (cachePath_.empty() || !std::filesystem::exists(cachePath_)) return;

    try {
        std::ifstream in(cachePath_);
        nlohmann::json j;
        in >> j;

        std::lock_guard lock(mutex_);
        accountProgress_.clear();
        progress_.clear();

        if (j.contains("accounts") && j["accounts"].is_object()) {
            for (const auto& [account, data] : j["accounts"].items()) {
                accountProgress_[account] =
                    ProgressFromJson(data.value("achievements", nlohmann::json::array()),
                                     mentorMaxById_);
            }
            return;
        }

        if (j.contains("achievements")) {
            accountProgress_[""] =
                ProgressFromJson(j["achievements"], mentorMaxById_);
        }
    } catch (...) {
    }
}

void MentorAchievementProgressService::SaveCache() const {
    if (cachePath_.empty()) return;

    nlohmann::json j;
    j["version"] = "2.0";
    j["accounts"] = nlohmann::json::object();

    {
        std::lock_guard lock(mutex_);
        auto merged = accountProgress_;
        if (!activeAccount_.empty()) {
            merged[activeAccount_] = progress_;
        }

        for (const auto& [account, progress] : merged) {
            j["accounts"][account] = {{"achievements", ProgressToJson(progress)}};
        }
    }

    std::error_code ec;
    const auto parent = std::filesystem::path(cachePath_).parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
    }

    std::ofstream out(cachePath_);
    out << j.dump(2);
}

void MentorAchievementProgressService::LoadAccountSlice(const std::string& accountName) {
    SetActiveAccount(accountName);
}

void MentorAchievementProgressService::SaveAccountSlice(const std::string& accountName) const {
    (void)accountName;
    SaveCache();
}

std::vector<MentorProgressChange> MentorAchievementProgressService::RefreshFromApi(
    Gw2ApiClient& api, bool enabled) {
    std::vector<MentorProgressChange> increases;
    if (!enabled || mentorMaxById_.empty()) return increases;

    const auto token = api.FetchTokenInfo();
    if (!token.valid) return increases;

    bool hasAccount = false;
    bool hasProgression = false;
    for (const auto& perm : token.permissions) {
        if (perm == "account") hasAccount = true;
        if (perm == "progression") hasProgression = true;
    }
    if (!hasAccount || !hasProgression) return increases;

    const auto achievements = api.FetchAccountAchievements();
    if (!achievements) return increases;

    std::unordered_map<int, MentorProgressEntry> updated;
    for (const auto& ach : *achievements) {
        if (mentorMaxById_.count(ach.id) == 0) continue;
        MentorProgressEntry entry;
        entry.id = ach.id;
        entry.current = ach.current;
        entry.max = mentorMaxById_.at(ach.id);
        entry.done = ach.done;
        updated[ach.id] = entry;
    }

    bool changed = false;
    {
        std::lock_guard lock(mutex_);
        if (progress_.size() != updated.size()) {
            changed = true;
        } else {
            for (const auto& [id, entry] : updated) {
                const auto it = progress_.find(id);
                if (it == progress_.end() || it->second.current != entry.current ||
                    it->second.max != entry.max || it->second.done != entry.done) {
                    changed = true;
                    break;
                }
            }
        }

        if (changed) {
            for (const auto& [id, entry] : updated) {
                const auto it = progress_.find(id);
                if (it != progress_.end() && entry.current > it->second.current) {
                    increases.push_back({id, it->second.current, entry.current});
                }
            }

            progress_ = std::move(updated);
            if (!activeAccount_.empty()) {
                accountProgress_[activeAccount_] = progress_;
            }
        }
    }

    if (changed) {
        SaveCache();
    }

    return increases;
}

std::optional<MentorProgressEntry> MentorAchievementProgressService::GetProgress(
    int achievementId) const {
    std::lock_guard lock(mutex_);
    const auto it = progress_.find(achievementId);
    if (it == progress_.end()) return std::nullopt;
    return it->second;
}

int MentorAchievementProgressService::GetMaxForAchievement(int achievementId) const {
    const auto it = mentorMaxById_.find(achievementId);
    if (it == mentorMaxById_.end()) return kDefaultMax;
    return it->second;
}

}  // namespace rc
