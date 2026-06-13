#include "ui/RaidPanel.h"

#include "core/AppState.h"
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
    const auto raidPlacement =
        GridLayout::ComputePlacement(state.raidGroups, state.settings.panelLayout);
    const auto strikePlacement =
        GridLayout::ComputePlacement(state.strikeGroups, state.settings.panelLayout);

    if (state.settings.anchorStrikesToRaidPanel &&
        !OverlayPanel::IsDragging(OverlayPanel::PanelRole::Strikes)) {
        PanelAnchor::AlignStrikesToRaid(state.settings, raidPlacement.contentSize);
    }

    if (!OverlayPanel::Begin("Strike Clears", state.settings.strikesPanel,
                             strikePlacement.contentSize, OverlayPanel::PanelRole::Strikes)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridRenderer::DrawGroups(state.strikeGroups, state.settings, true, true, font);
    if (OverlayPanel::End(OverlayPanel::PanelRole::Strikes)) {
        PanelAnchor::OnStrikesDragged(state.settings, raidPlacement.contentSize);
    }
}

}  // namespace StrikesPanel
}  // namespace rc
