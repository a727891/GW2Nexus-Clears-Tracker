#include "ui/RaidPanel.h"

#include "core/AppState.h"
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
    const auto placement =
        GridLayout::ComputePlacement(state.raidGroups, state.settings.panelLayout);
    if (!OverlayPanel::Begin("Raid Clears", state.settings.raidPanel, placement.contentSize,
                             OverlayPanel::PanelRole::Raid)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridRenderer::DrawGroups(state.raidGroups, state.settings, true, true, font);
    if (OverlayPanel::End(OverlayPanel::PanelRole::Raid)) {
        PanelAnchor::OnRaidDragged(state.settings, placement.contentSize);
    }
}

}  // namespace RaidPanel
}  // namespace rc
