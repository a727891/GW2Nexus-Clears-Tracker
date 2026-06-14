#include "ui/RaidPanel.h"

#include "core/AppState.h"
#include "core/EncounterVisibilityFilter.h"
#include "ui/GridLayout.h"
#include "ui/GridRenderer.h"
#include "ui/OverlayPanel.h"
#include "ui/PanelAnchor.h"
#include "ui/UiFontService.h"

namespace rc {
namespace StrikesPanel {

void Render(AppState& state) {
    if (!state.ShouldShowPanel(state.settings.strikesPanel.visible)) return;
    if (!state.staticDataReady) return;

    std::lock_guard lock(state.dataMutex);
    const auto visibleRaidGroups =
        EncounterVisibilityFilter::FilterRaidGroups(state.raidGroups, state.raidVisibility);
    const auto visibleStrikeGroups = EncounterVisibilityFilter::FilterStrikeGroups(
        state.strikeGroups, state.strikeVisibility);
    const auto raidPlacement = GridLayout::ComputePlacement(
        visibleRaidGroups, state.settings.panelLayout, state.settings.panelScale,
        state.settings.groupLabelDisplay);
    const auto strikePlacement = GridLayout::ComputePlacement(
        visibleStrikeGroups, state.settings.panelLayout, state.settings.panelScale,
        state.settings.groupLabelDisplay);

    if (state.settings.anchorStrikesToRaidPanel &&
        !OverlayPanel::IsDragging(OverlayPanel::PanelRole::Strikes)) {
        PanelAnchor::AlignStrikesToRaid(state.settings, raidPlacement.contentSize);
    }

    if (!OverlayPanel::Begin("Raid Encounter Clears", state.settings.strikesPanel,
                             strikePlacement.contentSize, OverlayPanel::PanelRole::Strikes,
                             state.settings.lockPanelPosition)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridDrawContext context{&state.raidData, &state.strikeData, &state.mentorProgress, true};
    GridRenderer::DrawGroups(visibleStrikeGroups, state.settings, true, true, font, context);
    const uint32_t screenW = state.nexusLink ? state.nexusLink->Width : 0;
    const uint32_t screenH = state.nexusLink ? state.nexusLink->Height : 0;
    if (OverlayPanel::End(OverlayPanel::PanelRole::Strikes, state.settings.screenClamp, screenW,
                          screenH)) {
        PanelAnchor::OnStrikesDragged(state.settings, raidPlacement.contentSize,
                                       strikePlacement.contentSize);
    }
}

}  // namespace StrikesPanel
}  // namespace rc
