#include "ui/RaidPanel.h"

#include "core/AppState.h"
#include "ui/GridRenderer.h"

#include <imgui.h>

namespace rc {
namespace StrikesPanel {

void Render(AppState& state) {
    if (!state.ShouldShowPanel(state.settings.strikesPanel.visible)) return;
    if (!state.staticDataReady) return;

    ImGui::SetNextWindowPos(
        ImVec2(state.settings.strikesPanel.posX, state.settings.strikesPanel.posY),
        ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.35f);

    if (!ImGui::Begin("Strike Clears", &state.settings.strikesPanel.visible,
                      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
                          ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return;
    }

    const ImVec2 pos = ImGui::GetWindowPos();
    state.settings.strikesPanel.posX = pos.x;
    state.settings.strikesPanel.posY = pos.y;

    std::lock_guard lock(state.dataMutex);
    GridRenderer::DrawGroups(state.strikeGroups, state.settings, true);
    ImGui::End();
}

}  // namespace StrikesPanel
}  // namespace rc
