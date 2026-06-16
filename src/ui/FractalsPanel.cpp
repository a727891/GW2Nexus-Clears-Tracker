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

    std::vector<GridGroup> visibleRaidGroups;
    std::vector<GridGroup> visibleStrikeGroups;
    std::vector<GridGroup> fractalGroups;
    ImVec2 raidContentSize;
    ImVec2 strikeContentSize;
    ImVec2 fractalContentSize;
    const bool instabilitiesReady = state.instabilitiesDataReady;
    {
        std::lock_guard lock(state.dataMutex);
        visibleRaidGroups =
            EncounterVisibilityFilter::FilterRaidGroups(state.raidGroups, state.raidVisibility);
        visibleStrikeGroups = EncounterVisibilityFilter::FilterStrikeGroups(
            state.strikeGroups, state.strikeVisibility);
        fractalGroups = state.fractalGroups;
        raidContentSize = GridLayout::ComputePlacement(
                              visibleRaidGroups, state.settings.panelLayout,
                              state.settings.panelScale, state.settings.groupLabelDisplay)
                              .contentSize;
        strikeContentSize = GridLayout::ComputePlacement(
                                visibleStrikeGroups, state.settings.panelLayout,
                                state.settings.panelScale, state.settings.groupLabelDisplay)
                                .contentSize;
        fractalContentSize = GridLayout::ComputePlacement(
                                 fractalGroups, state.settings.panelLayout,
                                 state.settings.panelScale, state.settings.groupLabelDisplay)
                                 .contentSize;
    }

    if (state.settings.anchorFractalsToStrikesPanel &&
        !OverlayPanel::IsDragging(OverlayPanel::PanelRole::Fractals)) {
        if (state.settings.anchorStrikesToRaidPanel &&
            !OverlayPanel::IsDragging(OverlayPanel::PanelRole::Strikes)) {
            PanelAnchor::AlignStrikesToRaid(state.settings, raidContentSize);
        }
        PanelAnchor::AlignFractalsToStrikes(state.settings, strikeContentSize);
    }

    if (!OverlayPanel::Begin("Fractal Clears", state.settings.fractalsPanel, fractalContentSize,
                             OverlayPanel::PanelRole::Fractals,
                             state.settings.lockPanelPosition)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridDrawContext context{.raidData = &state.raidData,
                            .strikeData = &state.strikeData,
                            .mentorProgress = &state.mentorProgress,
                            .fractalMapData = &state.fractalMapData,
                            .instabilitiesData = instabilitiesReady ? &state.instabilitiesData
                                                                    : nullptr,
                            .fractalPersist = &state.fractalPersist,
                            .isFractalsPanel = true};
    GridRenderer::DrawGroups(fractalGroups, state.settings, true, true, font, context);
    const uint32_t screenW = state.nexusLink ? state.nexusLink->Width : 0;
    const uint32_t screenH = state.nexusLink ? state.nexusLink->Height : 0;
    if (OverlayPanel::End(OverlayPanel::PanelRole::Fractals, state.settings.screenClamp, screenW,
                          screenH)) {
        PanelAnchor::OnFractalsDragged(state.settings, raidContentSize, strikeContentSize);
    }
}

}  // namespace FractalsPanel
}  // namespace rc
