#pragma once

#include "core/SettingsStore.h"
#include "core/Types.h"

namespace rc {

namespace GridRenderer {
    void DrawGroup(const GridGroup& group, const SettingsStore& settings, bool colorClears);
    void DrawGroups(const std::vector<GridGroup>& groups,
                    const SettingsStore& settings,
                    bool colorClears);
}

}  // namespace rc
