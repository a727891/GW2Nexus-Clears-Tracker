#include "services/StrikeClearsService.h"

#include <unordered_set>

namespace rc {

void StrikeClearsService::RefreshFromApi(const StrikeData& strikeData,
                                         const std::string& accountName,
                                         Gw2ApiClient& client,
                                         StrikePersistance& persistence) {
    if (strikeData.weeklyAchievementId == 0 ||
        strikeData.weeklyAchievementBitStrikeIds.empty() ||
        accountName.empty()) {
        return;
    }

    auto achievements = client.FetchAccountAchievements();
    if (!achievements) return;

    const AccountAchievement* weekly = nullptr;
    for (const auto& ach : *achievements) {
        if (ach.id == strikeData.weeklyAchievementId) {
            weekly = &ach;
            break;
        }
    }
    if (!weekly) return;

    const auto& mapping = strikeData.weeklyAchievementBitStrikeIds;
    if (weekly->done) {
        for (const auto& strikeId : mapping) {
            persistence.SaveClear(accountName, strikeId);
        }
    } else {
        std::unordered_set<int> completedBits(weekly->bits.begin(), weekly->bits.end());
        for (size_t i = 0; i < mapping.size(); ++i) {
            if (completedBits.count(static_cast<int>(i)) > 0) {
                persistence.SaveClear(accountName, mapping[i]);
            } else {
                persistence.RemoveClear(accountName, mapping[i]);
            }
        }
    }
}

}  // namespace rc
