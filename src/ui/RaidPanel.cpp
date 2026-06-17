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

    const auto& appearance = state.settings.Appearance(PanelKind::Raids);
    std::vector<GridGroup> visibleGroups;
    ImVec2 contentSize;
    {
        std::lock_guard lock(state.dataMutex);
        visibleGroups =
            EncounterVisibilityFilter::FilterRaidGroups(state.raidGroups, state.raidVisibility);
        contentSize = GridLayout::ComputePlacement(visibleGroups, appearance.panelLayout,
                                                   appearance.panelScale, appearance.groupLabelDisplay)
                          .contentSize;
    }

    if (!OverlayPanel::Begin("Raid Clears", state.settings.raidPanel, contentSize,
                             OverlayPanel::PanelRole::Raid, state.settings.lockPanelPosition)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridDrawContext context{.raidData = &state.raidData,
                            .mentorProgress = &state.mentorProgress};
    GridRenderer::DrawGroups(visibleGroups, state.settings, appearance,
                             state.settings.raidEnableTooltips, true, true, font, context);
    const uint32_t screenW = state.nexusLink ? state.nexusLink->Width : 0;
    const uint32_t screenH = state.nexusLink ? state.nexusLink->Height : 0;
    if (OverlayPanel::End(OverlayPanel::PanelRole::Raid, state.settings.screenClamp, screenW,
                          screenH)) {
        ImVec2 strikeSize{};
        if (state.settings.anchorStrikesToRaidPanel ||
            state.settings.anchorFractalsToStrikesPanel) {
            const auto& strikeAppearance = state.settings.Appearance(PanelKind::Strikes);
            std::lock_guard lock(state.dataMutex);
            const auto visibleStrikeGroups = EncounterVisibilityFilter::FilterStrikeGroups(
                state.strikeGroups, state.strikeVisibility);
            strikeSize = GridLayout::ComputePlacement(visibleStrikeGroups,
                                                      strikeAppearance.panelLayout,
                                                      strikeAppearance.panelScale,
                                                      strikeAppearance.groupLabelDisplay)
                             .contentSize;
        }
        PanelAnchor::OnRaidDragged(state.settings, contentSize, strikeSize);
    }
}

}  // namespace RaidPanel
}  // namespace rc
