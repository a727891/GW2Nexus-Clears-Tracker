#include "ui/DungeonsPanel.h"

#include "core/AppState.h"
#include "ui/GridLayout.h"
#include "ui/GridRenderer.h"
#include "ui/OverlayPanel.h"
#include "ui/UiFontService.h"

namespace rc {
namespace DungeonsPanel {

namespace {

std::vector<GridGroup> FilterDungeonGroups(const std::vector<GridGroup>& groups,
                                           const SettingsStore& settings) {
    std::vector<GridGroup> visible;
    for (size_t i = 0; i < groups.size(); ++i) {
        if (groups[i].isFrequenterSummary) {
            if (settings.dungeonFrequenterVisible) {
                visible.push_back(groups[i]);
            }
            continue;
        }
        if (i < settings.dungeonVisible.size() && settings.dungeonVisible[i]) {
            visible.push_back(groups[i]);
        }
    }
    return visible;
}

}  // namespace

void Render(AppState& state) {
    if (!state.ShouldShowPanel(state.settings.dungeonsPanel.visible)) return;

    std::lock_guard lock(state.dataMutex);
    const auto visibleGroups = FilterDungeonGroups(state.dungeonGroups, state.settings);
    const auto placement = GridLayout::ComputePlacement(
        visibleGroups, state.settings.panelLayout, state.settings.panelScale);

    if (!OverlayPanel::Begin("Dungeon Clears", state.settings.dungeonsPanel, placement.contentSize,
                             OverlayPanel::PanelRole::Dungeons,
                             state.settings.lockPanelPosition)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridDrawContext context{&state.raidData, &state.strikeData, &state.mentorProgress, false};
    GridRenderer::DrawGroups(visibleGroups, state.settings, true, true, font, context, true);
    OverlayPanel::End(OverlayPanel::PanelRole::Dungeons);
}

}  // namespace DungeonsPanel
}  // namespace rc
