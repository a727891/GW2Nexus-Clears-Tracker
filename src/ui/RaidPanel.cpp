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
                                                        state.settings.panelScale);
    if (!OverlayPanel::Begin("Raid Clears", state.settings.raidPanel, placement.contentSize,
                             OverlayPanel::PanelRole::Raid)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridRenderer::DrawGroups(visibleGroups, state.settings, true, true, font, &state.raidData);
    if (OverlayPanel::End(OverlayPanel::PanelRole::Raid)) {
        PanelAnchor::OnRaidDragged(state.settings, placement.contentSize);
    }
}

}  // namespace RaidPanel
}  // namespace rc
