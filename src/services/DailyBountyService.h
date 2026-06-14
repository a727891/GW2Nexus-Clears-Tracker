#pragma once

#include "core/Types.h"
#include "data/DailyBountyData.h"
#include "data/RaidData.h"
#include "data/StrikeData.h"

#include <vector>

namespace rc {

class RaidVisibilityStore;
class StrikeVisibilityStore;

class DailyBountyService {
public:
    static std::vector<std::string> GetBountyEncounterApiIdsForDay(const DailyBountyData& bountyData,
                                                                   int dayIndex);

    static std::vector<EncounterCell> GetDailyBounties(const DailyBountyData& bountyData,
                                                       const RaidData& raidData,
                                                       const StrikeData& strikeData,
                                                       int dayOffset = 0,
                                                       const RaidVisibilityStore* raidLabels = nullptr,
                                                       const StrikeVisibilityStore* strikeLabels = nullptr);
};

}  // namespace rc
