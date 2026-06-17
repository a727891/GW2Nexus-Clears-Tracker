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

    const auto& raidAppearance = state.settings.Appearance(PanelKind::Raids);
    const auto& strikeAppearance = state.settings.Appearance(PanelKind::Strikes);
    std::vector<GridGroup> visibleRaidGroups;
    std::vector<GridGroup> visibleStrikeGroups;
    ImVec2 raidContentSize;
    ImVec2 strikeContentSize;
    {
        std::lock_guard lock(state.dataMutex);
        visibleRaidGroups =
            EncounterVisibilityFilter::FilterRaidGroups(state.raidGroups, state.raidVisibility);
        visibleStrikeGroups = EncounterVisibilityFilter::FilterStrikeGroups(
            state.strikeGroups, state.strikeVisibility);
        raidContentSize = GridLayout::ComputePlacement(
                              visibleRaidGroups, raidAppearance.panelLayout,
                              raidAppearance.panelScale, raidAppearance.groupLabelDisplay)
                              .contentSize;
        strikeContentSize = GridLayout::ComputePlacement(
                                visibleStrikeGroups, strikeAppearance.panelLayout,
                                strikeAppearance.panelScale, strikeAppearance.groupLabelDisplay)
                                .contentSize;
    }

    if (state.settings.anchorStrikesToRaidPanel &&
        !OverlayPanel::IsDragging(OverlayPanel::PanelRole::Strikes)) {
        PanelAnchor::AlignStrikesToRaid(state.settings, raidContentSize);
    }

    if (!OverlayPanel::Begin("Raid Encounter Clears", state.settings.strikesPanel,
                             strikeContentSize, OverlayPanel::PanelRole::Strikes,
                             state.settings.lockPanelPosition)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridDrawContext context{.raidData = &state.raidData,
                            .strikeData = &state.strikeData,
                            .mentorProgress = &state.mentorProgress,
                            .isStrikePanel = true};
    GridRenderer::DrawGroups(visibleStrikeGroups, state.settings, strikeAppearance,
                             state.settings.strikesEnableTooltips, true, true, font, context);
    const uint32_t screenW = state.nexusLink ? state.nexusLink->Width : 0;
    const uint32_t screenH = state.nexusLink ? state.nexusLink->Height : 0;
    if (OverlayPanel::End(OverlayPanel::PanelRole::Strikes, state.settings.screenClamp, screenW,
                          screenH)) {
        PanelAnchor::OnStrikesDragged(state.settings, raidContentSize, strikeContentSize);
    }
}

}  // namespace StrikesPanel
}  // namespace rc
