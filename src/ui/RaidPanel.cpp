#include "ui/RaidPanel.h"

#include "core/AppState.h"
#include "core/EncounterVisibilityFilter.h"
#include "ui/GridLayout.h"
#include "ui/GridRenderer.h"
#include "ui/OverlayPanel.h"
#include "ui/PanelAnchor.h"
#include "ui/UiFontService.h"

namespace rc {
namespace RaidPanel {

void Render(AppState& state) {
    if (!state.ShouldShowPanel(state.settings.raidPanel.visible)) return;
    if (!state.staticDataReady) return;

    std::lock_guard lock(state.dataMutex);
    const auto visibleGroups =
        EncounterVisibilityFilter::FilterRaidGroups(state.raidGroups, state.raidVisibility);
    const auto placement = GridLayout::ComputePlacement(visibleGroups, state.settings.panelLayout,
                                                        state.settings.panelScale,
                                                        state.settings.groupLabelDisplay);
    if (!OverlayPanel::Begin("Raid Clears", state.settings.raidPanel, placement.contentSize,
                             OverlayPanel::PanelRole::Raid, state.settings.lockPanelPosition)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridDrawContext context{.raidData = &state.raidData,
                            .mentorProgress = &state.mentorProgress};
    GridRenderer::DrawGroups(visibleGroups, state.settings, true, true, font, context);
    const uint32_t screenW = state.nexusLink ? state.nexusLink->Width : 0;
    const uint32_t screenH = state.nexusLink ? state.nexusLink->Height : 0;
    if (OverlayPanel::End(OverlayPanel::PanelRole::Raid, state.settings.screenClamp, screenW,
                          screenH)) {
        ImVec2 strikeSize{};
        if (state.settings.anchorStrikesToRaidPanel ||
            state.settings.anchorFractalsToStrikesPanel) {
            const auto visibleStrikeGroups = EncounterVisibilityFilter::FilterStrikeGroups(
                state.strikeGroups, state.strikeVisibility);
            strikeSize = GridLayout::ComputePlacement(visibleStrikeGroups,
                                                      state.settings.panelLayout,
                                                      state.settings.panelScale,
                                                      state.settings.groupLabelDisplay)
                             .contentSize;
        }
        PanelAnchor::OnRaidDragged(state.settings, placement.contentSize, strikeSize);
    }
}

}  // namespace RaidPanel
}  // namespace rc
