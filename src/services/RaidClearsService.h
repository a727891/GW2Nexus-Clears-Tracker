#pragma once

#include "core/Types.h"
#include <unordered_set>
#include <vector>

namespace rc {

class RaidClearsService {
public:
    static void ApplyClears(std::vector<GridGroup>& groups,
                            const std::unordered_set<std::string>& clears);
};

}  // namespace rc
