#pragma once

#include "core/RaidVisibilityStore.h"
#include "core/StrikeVisibilityStore.h"
#include "core/Types.h"

#include <vector>

namespace rc {
namespace EncounterVisibilityFilter {

std::vector<GridGroup> FilterRaidGroups(const std::vector<GridGroup>& groups,
                                        const RaidVisibilityStore& visibility);

std::vector<GridGroup> FilterStrikeGroups(const std::vector<GridGroup>& groups,
                                          const StrikeVisibilityStore& visibility);

}  // namespace EncounterVisibilityFilter
}  // namespace rc
