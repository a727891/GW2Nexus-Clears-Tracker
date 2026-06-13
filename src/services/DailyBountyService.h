#pragma once

#include "data/DailyBountyData.h"
#include "data/RaidData.h"
#include "data/StrikeData.h"
#include "core/Types.h"
#include <vector>

namespace rc {

class DailyBountyService {
public:
    static std::vector<std::string> GetBountyEncounterApiIdsForDay(const DailyBountyData& bountyData,
                                                                   int dayIndex);

    static std::vector<EncounterCell> GetDailyBounties(const DailyBountyData& bountyData,
                                                     const RaidData& raidData,
                                                     const StrikeData& strikeData,
                                                     int dayOffset = 0);
};

}  // namespace rc
