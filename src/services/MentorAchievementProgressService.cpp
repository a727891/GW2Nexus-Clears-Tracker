#include "services/MentorAchievementProgressService.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace rc {

void MentorAchievementProgressService::Initialize(const RaidData& raidData,
                                                  const std::string& cachePath) {
    cachePath_ = cachePath;
    mentorMaxById_.clear();
    {
        std::lock_guard lock(mutex_);
        progress_.clear();
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

void MentorAchievementProgressService::LoadCache() {
    if (cachePath_.empty() || !std::filesystem::exists(cachePath_)) return;

    try {
        std::ifstream in(cachePath_);
        nlohmann::json j;
        in >> j;
        if (!j.contains("achievements")) return;

        std::unordered_map<int, MentorProgressEntry> loaded;
        for (const auto& entryJ : j["achievements"]) {
            MentorProgressEntry entry;
            entry.id = entryJ.value("id", 0);
            entry.current = entryJ.value("current", 0);
            entry.max = entryJ.value("max", kDefaultMax);
            entry.done = entryJ.value("done", false);
            if (mentorMaxById_.count(entry.id) == 0) continue;
            entry.max = mentorMaxById_.at(entry.id);
            loaded[entry.id] = entry;
        }

        std::lock_guard lock(mutex_);
        progress_ = std::move(loaded);
    } catch (...) {
    }
}

void MentorAchievementProgressService::SaveCache() const {
    if (cachePath_.empty()) return;

    nlohmann::json j;
    j["version"] = "1.0";
    j["achievements"] = nlohmann::json::array();

    {
        std::lock_guard lock(mutex_);
        for (const auto& [id, entry] : progress_) {
            (void)id;
            j["achievements"].push_back(
                {{"id", entry.id},
                 {"current", entry.current},
                 {"max", entry.max},
                 {"done", entry.done}});
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

void MentorAchievementProgressService::RefreshFromApi(Gw2ApiClient& api, bool enabled) {
    if (!enabled || mentorMaxById_.empty()) return;

    const auto token = api.ValidateToken();
    if (!token.valid) return;

    bool hasAccount = false;
    bool hasProgression = false;
    for (const auto& perm : token.permissions) {
        if (perm == "account") hasAccount = true;
        if (perm == "progression") hasProgression = true;
    }
    if (!hasAccount || !hasProgression) return;

    const auto achievements = api.FetchAccountAchievements();
    if (!achievements) return;

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
            progress_ = std::move(updated);
        }
    }

    if (changed) {
        SaveCache();
    }
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
