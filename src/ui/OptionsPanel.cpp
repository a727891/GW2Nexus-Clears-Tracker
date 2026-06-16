#include "ui/SettingsWindow.h"

#include "core/AppState.h"
#include "core/Branding.h"
#include "ui/ContentLogoService.h"
#include "ui/OptionsPanel.h"
#include "ui/OptionsUiKit.h"

#include <imgui.h>

namespace rc {
namespace OptionsPanel {
namespace {

ImGuiTabItemFlags PendingTabFlags(SettingsTab tab, SettingsTab pendingTab, bool applyPendingTab) {
    if (applyPendingTab && tab == pendingTab) {
        return ImGuiTabItemFlags_SetSelected;
    }
    return ImGuiTabItemFlags_None;
}

}  // namespace

void RenderNexusConfigEntry(AppState& state) {
    using namespace OptionsUiKit;

    (void)state;

    ImGui::TextColored(GoldColor(), "%s", kDisplayName);
    ImGui::Spacing();
    ImGui::TextWrapped("%s", kDescription);
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Configure raid, strike, fractal, and dungeon panels, API keys, and quick access from a "
        "dedicated in-game settings window.");
    ImGui::Spacing();
    ImGui::Spacing();

    const ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 52.0f);
    if (ImGui::Button("Open Clears Tracker Settings", buttonSize)) {
        SettingsWindow::Open();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Opens the full settings window.");
    }
}

void RenderWindow(AppState& state, SettingsTab pendingTab, bool applyPendingTab) {
    using namespace OptionsUiKit;

    static ShellState shell;

    if (state.api) {
        ContentLogoService::Initialize(state.api, state.addonDir);
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
        if (ImGui::BeginTabItem("General", nullptr,
                                PendingTabFlags(SettingsTab::General, pendingTab,
                                                applyPendingTab))) {
            OptionsGeneralTab::Render(state, shell.generalSection);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Raids", nullptr,
                                PendingTabFlags(SettingsTab::Raids, pendingTab, applyPendingTab))) {
            OptionsRaidsTab::Render(state, shell.raidsSection);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Raid Encounters", nullptr,
                                PendingTabFlags(SettingsTab::Strikes, pendingTab,
                                                applyPendingTab))) {
            OptionsStrikesTab::Render(state, shell.strikesSection);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fractals", nullptr,
                                PendingTabFlags(SettingsTab::Fractals, pendingTab,
                                                applyPendingTab))) {
            OptionsFractalsTab::Render(state, shell.fractalsSection);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Dungeons", nullptr,
                                PendingTabFlags(SettingsTab::Dungeons, pendingTab,
                                                applyPendingTab))) {
            OptionsDungeonsTab::Render(state, shell.dungeonsSection);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

}  // namespace OptionsPanel
}  // namespace rc
