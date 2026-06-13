#include "services/DailyBountyProgressService.h"

namespace rc {

void DailyBountyProgressService::RefreshFromApi(Gw2ApiClient& client,
                                                const std::string& categoryUrl) {
    std::unordered_set<int> completed;
    const auto categoryIds = client.FetchAchievementCategoryIds(categoryUrl);
    if (!categoryIds || categoryIds->empty()) {
        std::lock_guard lock(mutex_);
        completedIds_.clear();
        return;
    }

    const auto achievements = client.FetchAccountAchievements();
    if (!achievements) {
        std::lock_guard lock(mutex_);
        completedIds_.clear();
        return;
    }

    std::unordered_set<int> bountySet(categoryIds->begin(), categoryIds->end());
    for (const auto& ach : *achievements) {
        if (bountySet.count(ach.id) > 0 && ach.done) {
            completed.insert(ach.id);
        }
    }

    std::lock_guard lock(mutex_);
    completedIds_ = std::move(completed);
}

std::unordered_set<int> DailyBountyProgressService::GetCompletedIds() const {
    std::lock_guard lock(mutex_);
    return completedIds_;
}

}  // namespace rc
