#include "ui/FractalsPanel.h"

#include "core/AppState.h"
#include "core/EncounterVisibilityFilter.h"
#include "ui/GridLayout.h"
#include "ui/GridRenderer.h"
#include "ui/OverlayPanel.h"
#include "ui/PanelAnchor.h"
#include "ui/UiFontService.h"

namespace rc {
namespace FractalsPanel {

void Render(AppState& state) {
    if (!state.ShouldShowPanel(state.settings.fractalsPanel.visible)) return;
    if (!state.fractalDataReady) return;

    std::lock_guard lock(state.dataMutex);
    const auto visibleRaidGroups =
        EncounterVisibilityFilter::FilterRaidGroups(state.raidGroups, state.raidVisibility);
    const auto visibleStrikeGroups = EncounterVisibilityFilter::FilterStrikeGroups(
        state.strikeGroups, state.strikeVisibility);
    const auto raidPlacement = GridLayout::ComputePlacement(
        visibleRaidGroups, state.settings.panelLayout, state.settings.panelScale);
    const auto strikePlacement = GridLayout::ComputePlacement(
        visibleStrikeGroups, state.settings.panelLayout, state.settings.panelScale);
    const auto fractalPlacement = GridLayout::ComputePlacement(
        state.fractalGroups, state.settings.panelLayout, state.settings.panelScale);

    if (state.settings.anchorFractalsToStrikesPanel &&
        !OverlayPanel::IsDragging(OverlayPanel::PanelRole::Fractals)) {
        if (state.settings.anchorStrikesToRaidPanel &&
            !OverlayPanel::IsDragging(OverlayPanel::PanelRole::Strikes)) {
            PanelAnchor::AlignStrikesToRaid(state.settings, raidPlacement.contentSize);
        }
        PanelAnchor::AlignFractalsToStrikes(state.settings, strikePlacement.contentSize);
    }

    if (!OverlayPanel::Begin("Fractal Clears", state.settings.fractalsPanel,
                             fractalPlacement.contentSize, OverlayPanel::PanelRole::Fractals,
                             state.settings.lockPanelPosition)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridDrawContext context{&state.raidData, &state.strikeData, &state.mentorProgress, false};
    GridRenderer::DrawGroups(state.fractalGroups, state.settings, true, true, font, context);
    if (OverlayPanel::End(OverlayPanel::PanelRole::Fractals)) {
        PanelAnchor::OnFractalsDragged(state.settings, raidPlacement.contentSize,
                                       strikePlacement.contentSize);
    }
}

}  // namespace FractalsPanel
}  // namespace rc
