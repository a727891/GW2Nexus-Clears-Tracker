#include "ui/OptionsCommon.h"

#include "core/AppState.h"
#include "core/EncounterVisibilityFilter.h"
#include "ui/GridLayout.h"
#include "ui/PanelAnchor.h"

#include <mutex>

namespace rc {

void RealignAnchoredPanels(AppState& state) {
    if (!state.staticDataReady) return;
    std::lock_guard lock(state.dataMutex);
    const auto raidSize =
        GridLayout::ComputePlacement(
            EncounterVisibilityFilter::FilterRaidGroups(state.raidGroups, state.raidVisibility),
            state.settings.panelLayout, state.settings.panelScale,
            state.settings.groupLabelDisplay)
            .contentSize;
    const auto strikeSize =
        GridLayout::ComputePlacement(
            EncounterVisibilityFilter::FilterStrikeGroups(state.strikeGroups,
                                                          state.strikeVisibility),
            state.settings.panelLayout, state.settings.panelScale,
            state.settings.groupLabelDisplay)
            .contentSize;
    if (state.settings.anchorStrikesToRaidPanel) {
        PanelAnchor::AlignStrikesToRaid(state.settings, raidSize);
    }
    if (state.settings.anchorFractalsToStrikesPanel) {
        PanelAnchor::AlignFractalsToStrikes(state.settings, strikeSize);
    }
}

}  // namespace rc
