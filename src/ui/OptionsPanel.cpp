#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "core/Branding.h"
#include "ui/OptionsTextureService.h"
#include "ui/OptionsUiKit.h"

#include <imgui.h>

namespace rc {
namespace OptionsPanel {

void Render(AppState& state) {
    using namespace OptionsUiKit;

    static ShellState shell;

    if (state.api) {
        OptionsTextureService::Initialize(state.api, state.addonDir);
        OptionsTextureService::RequestAssets();
    }

    ImGui::TextColored(GoldColor(), kDisplayName);
    ImGui::Separator();
    ImGui::Spacing();

    OptionsUiKit::SettingCheckbox("Lock panel position", &state.settings.lockPanelPosition,
                                  "Prevents dragging all four overlay panels. While locked, mouse "
                                  "input passes through the panels to the game. Tooltips still "
                                  "appear when hovering encounter cells.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##nrc_options_tabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("General")) {
            OptionsGeneralTab::Render(state, shell.generalSection);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Raids")) {
            OptionsRaidsTab::Render(state, shell.raidsSection);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Raid Encounters")) {
            OptionsStrikesTab::Render(state, shell.strikesSection);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fractals")) {
            OptionsFractalsTab::Render(state, shell.fractalsSection);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Dungeons")) {
            OptionsDungeonsTab::Render(state, shell.dungeonsSection);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

}  // namespace OptionsPanel
}  // namespace rc
