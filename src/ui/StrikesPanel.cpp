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
        visibleRaidGroups, state.settings.panelLayout, state.settings.panelScale);
    const auto strikePlacement = GridLayout::ComputePlacement(
        visibleStrikeGroups, state.settings.panelLayout, state.settings.panelScale);

    if (state.settings.anchorStrikesToRaidPanel &&
        !OverlayPanel::IsDragging(OverlayPanel::PanelRole::Strikes)) {
        PanelAnchor::AlignStrikesToRaid(state.settings, raidPlacement.contentSize);
    }

    if (!OverlayPanel::Begin("Strike Clears", state.settings.strikesPanel,
                             strikePlacement.contentSize, OverlayPanel::PanelRole::Strikes)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridRenderer::DrawGroups(visibleStrikeGroups, state.settings, true, true, font);
    if (OverlayPanel::End(OverlayPanel::PanelRole::Strikes)) {
        PanelAnchor::OnStrikesDragged(state.settings, raidPlacement.contentSize);
    }
}

}  // namespace StrikesPanel
}  // namespace rc
