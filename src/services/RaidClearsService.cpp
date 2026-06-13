#include "services/RaidClearsService.h"

namespace rc {

void RaidClearsService::ApplyClears(std::vector<GridGroup>& groups,
                                    const std::unordered_set<std::string>& clears) {
    for (auto& group : groups) {
        for (auto& enc : group.encounters) {
            if (clears.find(enc.id) != clears.end()) {
                enc.state = ClearState::Cleared;
            } else {
                enc.state = ClearState::NotCleared;
            }
        }
    }
}

}  // namespace rc
