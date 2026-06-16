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

    std::vector<GridGroup> visibleGroups;
    ImVec2 contentSize;
    {
        std::lock_guard lock(state.dataMutex);
        visibleGroups = FilterDungeonGroups(state.dungeonGroups, state.settings);
        contentSize = GridLayout::ComputePlacement(
                            visibleGroups, state.settings.panelLayout, state.settings.panelScale,
                            state.settings.groupLabelDisplay)
                            .contentSize;
    }

    if (!OverlayPanel::Begin("Dungeon Clears", state.settings.dungeonsPanel, contentSize,
                             OverlayPanel::PanelRole::Dungeons,
                             state.settings.lockPanelPosition)) {
        return;
    }

    ImFont* font = UiFontService::GetGridFont(state.nexusLink);
    GridDrawContext context{.raidData = &state.raidData,
                            .strikeData = &state.strikeData,
                            .mentorProgress = &state.mentorProgress};
    GridRenderer::DrawGroups(visibleGroups, state.settings, true, true, font, context, true);
    const uint32_t screenW = state.nexusLink ? state.nexusLink->Width : 0;
    const uint32_t screenH = state.nexusLink ? state.nexusLink->Height : 0;
    OverlayPanel::End(OverlayPanel::PanelRole::Dungeons, state.settings.screenClamp, screenW,
                      screenH);
}

}  // namespace DungeonsPanel
}  // namespace rc
