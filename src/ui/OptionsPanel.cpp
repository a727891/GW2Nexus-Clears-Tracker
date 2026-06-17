#include "ui/SettingsWindow.h"

#include "core/AppState.h"
#include "core/Branding.h"
#include "ui/ContentLogoService.h"
#include "ui/OptionsPanel.h"
#include "ui/OptionsUiKit.h"

#include <cstring>
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

    ImGui::TextColored(GoldColor(), "%s", kDisplayName);
    ImGui::Spacing();
    ImGui::TextWrapped("%s", kDescription);
    ImGui::Spacing();
    ImGui::TextWrapped("Register a GW2 API key to start tracking clears.");
    ImGui::Spacing();

    static char apiKeyBuf[128] = {};
    const bool registering = state.accountRegistry.IsRegistering();

    ImGui::InputText("GW2 API Key", apiKeyBuf, sizeof(apiKeyBuf), ImGuiInputTextFlags_Password);
    if (!registering && ImGui::Button("Register Key")) {
        if (apiKeyBuf[0] != '\0') {
            state.RegisterApiKey(apiKeyBuf);
            apiKeyBuf[0] = '\0';
            SettingsWindow::Open(SettingsTab::General, SettingsWindow::kGeneralSectionApiSync);
        }
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(
            "Validates the key against the GW2 API and stores account name, key name, and "
            "characters. Requires account, progression, and characters permissions.");
    }

    const auto status = state.accountRegistry.LastRegistrationMessage();
    if (!status.empty()) {
        ImGui::TextWrapped("%s", status.c_str());
    }
    if (registering) {
        ImGui::TextDisabled("Registering API key...");
    }

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

void RenderWindow(AppState& state, SettingsTab pendingTab, bool applyPendingTab,
                  int pendingGeneralSection, bool applyPendingGeneralSection) {
    using namespace OptionsUiKit;

    static ShellState shell;

    if (applyPendingGeneralSection) {
        shell.generalSection = pendingGeneralSection;
    }

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

    if (ImGui::BeginTabBar("##clrtrk_options_tabs", ImGuiTabBarFlags_None)) {
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
