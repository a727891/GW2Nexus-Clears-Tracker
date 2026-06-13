#include "core/EncounterVisibilityFilter.h"

#include <algorithm>

namespace rc {
namespace EncounterVisibilityFilter {

std::vector<GridGroup> FilterRaidGroups(const std::vector<GridGroup>& groups,
                                        const RaidVisibilityStore& visibility) {
    std::vector<GridGroup> filtered;
    filtered.reserve(groups.size());

    for (const auto& group : groups) {
        if (!visibility.IsWingVisible(group.id)) continue;

        GridGroup copy = group;
        copy.encounters.erase(std::remove_if(copy.encounters.begin(), copy.encounters.end(),
                                             [&](const EncounterCell& cell) {
                                                 return !visibility.IsEncounterVisible(cell.id);
                                             }),
                              copy.encounters.end());
        if (copy.encounters.empty()) continue;
        filtered.push_back(std::move(copy));
    }

    return filtered;
}

std::vector<GridGroup> FilterStrikeGroups(const std::vector<GridGroup>& groups,
                                            const StrikeVisibilityStore& visibility) {
    std::vector<GridGroup> filtered;
    filtered.reserve(groups.size());

    for (const auto& group : groups) {
        if (group.isDailyBounty) {
            if (!visibility.IsPriorityVisible()) continue;
            filtered.push_back(group);
            continue;
        }
        if (group.isTomorrowBounty) {
            if (!visibility.IsTomorrowBountiesVisible()) continue;
            filtered.push_back(group);
            continue;
        }

        if (!visibility.IsExpansionVisible(group.id)) continue;

        GridGroup copy = group;
        copy.encounters.erase(std::remove_if(copy.encounters.begin(), copy.encounters.end(),
                                             [&](const EncounterCell& cell) {
                                                 return !visibility.IsMissionVisible(cell.id);
                                             }),
                              copy.encounters.end());
        if (copy.encounters.empty()) continue;
        filtered.push_back(std::move(copy));
    }

    return filtered;
}

}  // namespace EncounterVisibilityFilter
}  // namespace rc
