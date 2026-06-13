#include "ui/RaidPanel.h"

#include "core/AppState.h"
#include "ui/GridRenderer.h"

#include <imgui.h>

namespace rc {
namespace RaidPanel {

void Render(AppState& state) {
    if (!state.ShouldShowPanel(state.settings.raidPanel.visible)) return;
    if (!state.staticDataReady) return;

    ImGui::SetNextWindowPos(ImVec2(state.settings.raidPanel.posX, state.settings.raidPanel.posY),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.35f);

    if (!ImGui::Begin("Raid Clears", &state.settings.raidPanel.visible,
                      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
                          ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return;
    }

    const ImVec2 pos = ImGui::GetWindowPos();
    state.settings.raidPanel.posX = pos.x;
    state.settings.raidPanel.posY = pos.y;

    std::lock_guard lock(state.dataMutex);
    GridRenderer::DrawGroups(state.raidGroups, state.settings, true);
    ImGui::End();
}

}  // namespace RaidPanel
}  // namespace rc
