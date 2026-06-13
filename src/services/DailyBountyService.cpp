#include "services/DailyBountyService.h"
#include "services/PriorityRotationService.h"

#include "data/StrikeData.h"

#include <optional>

namespace rc {

namespace {

std::vector<std::string> GetBountyApiIdsForDay(const DailyBountyData& bountyData, int dayIndex) {
    std::vector<std::string> ids;
    for (const auto& slot : bountyData.bossSlots) {
        if (slot.encounters.empty()) continue;
        const int modulo = static_cast<int>(slot.encounters.size());
        int index = (dayIndex + slot.offset) % modulo;
        if (index < 0) index += modulo;
        ids.push_back(slot.encounters[static_cast<size_t>(index)]);
    }
    return ids;
}

std::optional<EncounterCell> ResolveBountyEncounter(const std::string& apiId,
                                                    const RaidData& raidData,
                                                    const StrikeData& strikeData) {
    if (const auto* raidEnc = raidData.GetEncounterByApiId(apiId)) {
        EncounterCell cell;
        cell.id = apiId;
        cell.name = raidEnc->name;
        cell.abbreviation = raidEnc->abbreviation;
        cell.state = ClearState::Unknown;
        if (raidEnc->dailyBountyAchievementId) {
            cell.dailyBountyAchievementId = *raidEnc->dailyBountyAchievementId;
        }
        return cell;
    }

    if (const auto* strikeEnc = strikeData.GetMissionById(apiId)) {
        EncounterCell cell;
        cell.id = apiId;
        cell.name = strikeEnc->name;
        cell.abbreviation = strikeEnc->abbreviation;
        cell.state = ClearState::Unknown;
        if (strikeEnc->dailyBountyAchievementId) {
            cell.dailyBountyAchievementId = *strikeEnc->dailyBountyAchievementId;
        }
        return cell;
    }

    return std::nullopt;
}

}  // namespace

std::vector<EncounterCell> DailyBountyService::GetDailyBounties(const DailyBountyData& bountyData,
                                                                const RaidData& raidData,
                                                                const StrikeData& strikeData,
                                                                int dayOffset) {
    std::vector<EncounterCell> cells;
    if (!bountyData.enabled) return cells;

    const int dayIndex = DayOfYearIndex() + dayOffset;
    const auto apiIds = GetBountyApiIdsForDay(bountyData, dayIndex);

    for (const auto& apiId : apiIds) {
        if (auto cell = ResolveBountyEncounter(apiId, raidData, strikeData)) {
            cells.push_back(std::move(*cell));
        }
    }
    return cells;
}

}  // namespace rc
