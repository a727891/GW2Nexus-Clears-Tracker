#include "ui/RaidPanel.h"

#include "core/AppState.h"
#include "ui/GridLayout.h"
#include "ui/PanelAnchor.h"
#include "ui/QuickAccessService.h"

#include <cstring>
#include <imgui.h>

namespace rc {
namespace OptionsPanel {

void Render(AppState& state) {
    ImGui::Separator();
    ImGui::Text("Nexus Raid Clears");

    static char apiKeyBuf[128] = {};
    if (apiKeyBuf[0] == '\0' && !state.settings.apiKey.empty()) {
        strncpy(apiKeyBuf, state.settings.apiKey.c_str(), sizeof(apiKeyBuf) - 1);
    }

    ImGui::InputText("GW2 API Key", apiKeyBuf, sizeof(apiKeyBuf), ImGuiInputTextFlags_Password);
    if (ImGui::Button("Save API Key")) {
        state.settings.apiKey = apiKeyBuf;
        state.gw2Api.SetApiKey(state.settings.apiKey);
        const auto info = state.gw2Api.ValidateToken();
        if (state.api) {
            if (info.valid) {
                state.api->Log(LOGL_INFO, "NexusRaidClears", "API key saved and validated.");
            } else {
                state.api->Log(LOGL_WARNING, "NexusRaidClears", "API key saved but validation failed.");
            }
        }
        state.RequestApiRefresh();
    }

    ImGui::Checkbox("Show Raids Panel", &state.settings.raidPanel.visible);
    ImGui::Checkbox("Show Strikes Panel", &state.settings.strikesPanel.visible);

    const char* layoutLabels[] = {"Vertical", "Horizontal"};
    int layoutIndex = state.settings.panelLayout == PanelLayout::Horizontal ? 1 : 0;
    if (ImGui::Combo("Panel layout", &layoutIndex, layoutLabels, 2)) {
        state.settings.panelLayout =
            layoutIndex == 1 ? PanelLayout::Horizontal : PanelLayout::Vertical;
        if (state.settings.anchorStrikesToRaidPanel) {
            std::lock_guard lock(state.dataMutex);
            const auto raidSize = GridLayout::ComputePlacement(state.raidGroups,
                                                               state.settings.panelLayout)
                                      .contentSize;
            PanelAnchor::AlignStrikesToRaid(state.settings, raidSize);
        }
    }

    if (ImGui::Checkbox("Anchor strikes panel to raid panel",
                        &state.settings.anchorStrikesToRaidPanel)) {
        if (state.settings.anchorStrikesToRaidPanel) {
            std::lock_guard lock(state.dataMutex);
            const auto raidSize = GridLayout::ComputePlacement(state.raidGroups,
                                                               state.settings.panelLayout)
                                      .contentSize;
            PanelAnchor::AlignStrikesToRaid(state.settings, raidSize);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Keeps the strikes panel attached to the raid panel. Moving either panel moves "
            "both.");
    }

    ImGui::Text("Panel visibility shortcut (NRC_TOGGLE_PANELS)");
    ImGui::Checkbox("Toggle raids on keybind / corner icon click",
                    &state.settings.keybindToggleRaids);
    ImGui::Checkbox("Toggle strikes on keybind / corner icon click",
                    &state.settings.keybindToggleStrikes);

    if (ImGui::Checkbox("Show corner icon", &state.settings.cornerIconEnabled)) {
        if (state.api) {
            rc::QuickAccessService::SyncVisibility(state.api, state);
        }
    }

    if (ImGui::Checkbox("Highlight non-weekly bounty encounters",
                          &state.settings.highlightNonWeeklyBounty)) {
        std::lock_guard lock(state.dataMutex);
        state.ApplyNonWeeklyHighlights();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Uncleared raid and strike bosses that will not be daily bounties for the rest of "
            "the week are highlighted in orange.");
    }
    if (ImGui::Checkbox("Omit event encounters from highlight", &state.settings.omitEventEncounters)) {
        std::lock_guard lock(state.dataMutex);
        state.ApplyNonWeeklyHighlights();
    }

    int pollMinutes = state.settings.pollIntervalMinutes;
    if (ImGui::SliderInt("API Poll Interval (min)", &pollMinutes, 1, 30)) {
        state.settings.pollIntervalMinutes = pollMinutes;
        state.apiPoll.SetIntervalMinutes(pollMinutes);
    }

    float cleared[3] = {state.settings.colorCleared.r / 255.0f,
                        state.settings.colorCleared.g / 255.0f,
                        state.settings.colorCleared.b / 255.0f};
    float notCleared[3] = {state.settings.colorNotCleared.r / 255.0f,
                             state.settings.colorNotCleared.g / 255.0f,
                             state.settings.colorNotCleared.b / 255.0f};
    float unknown[3] = {state.settings.colorUnknown.r / 255.0f,
                        state.settings.colorUnknown.g / 255.0f,
                        state.settings.colorUnknown.b / 255.0f};

    if (ImGui::ColorEdit3("Cleared Color", cleared)) {
        state.settings.colorCleared = {static_cast<uint8_t>(cleared[0] * 255),
                                       static_cast<uint8_t>(cleared[1] * 255),
                                       static_cast<uint8_t>(cleared[2] * 255)};
    }
    if (ImGui::ColorEdit3("Not Cleared Color", notCleared)) {
        state.settings.colorNotCleared = {static_cast<uint8_t>(notCleared[0] * 255),
                                          static_cast<uint8_t>(notCleared[1] * 255),
                                          static_cast<uint8_t>(notCleared[2] * 255)};
    }
    if (ImGui::ColorEdit3("Unknown Color", unknown)) {
        state.settings.colorUnknown = {static_cast<uint8_t>(unknown[0] * 255),
                                       static_cast<uint8_t>(unknown[1] * 255),
                                       static_cast<uint8_t>(unknown[2] * 255)};
    }
    float nonWeekly[3] = {state.settings.colorNonWeeklyBounty.r / 255.0f,
                          state.settings.colorNonWeeklyBounty.g / 255.0f,
                          state.settings.colorNonWeeklyBounty.b / 255.0f};
    if (ImGui::ColorEdit3("Non-Weekly Bounty Color", nonWeekly)) {
        state.settings.colorNonWeeklyBounty = {
            static_cast<uint8_t>(nonWeekly[0] * 255),
            static_cast<uint8_t>(nonWeekly[1] * 255),
            static_cast<uint8_t>(nonWeekly[2] * 255)};
    }

    if (ImGui::Button("Refresh Now")) {
        state.RequestApiRefresh();
    }

    if (!state.accountName.empty()) {
        ImGui::Text("Account: %s", state.accountName.c_str());
    }
}

}  // namespace OptionsPanel
}  // namespace rc
