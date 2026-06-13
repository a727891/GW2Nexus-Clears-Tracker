#pragma once

#include "data/DailyBountyData.h"
#include "services/ResetsWatcher.h"

#include <mutex>
#include <string>
#include <unordered_set>

namespace rc {

class WeeklyBountyEncountersService {
public:
    void Rebuild(const DailyBountyData& bountyData, const ResetsWatcher& resets);
    bool IsWeeklyBounty(const std::string& encounterApiId) const;

private:
    mutable std::mutex mutex_;
    std::unordered_set<std::string> weeklyBountyApiIds_;
};

}  // namespace rc
