#pragma once

#include "services/Gw2ApiClient.h"
#include <mutex>
#include <unordered_set>

namespace rc {

class DailyBountyProgressService {
public:
    void RefreshFromApi(Gw2ApiClient& client, const std::string& categoryUrl);
    std::unordered_set<int> GetCompletedIds() const;

private:
    mutable std::mutex mutex_;
    std::unordered_set<int> completedIds_;
};

}  // namespace rc
