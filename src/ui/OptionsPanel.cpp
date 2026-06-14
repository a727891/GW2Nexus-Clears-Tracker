#include "ui/RaidPanel.h"

#include "core/AppState.h"
#include "core/EncounterVisibilityFilter.h"
#include "ui/EncounterSelectionPanel.h"
#include "ui/GridLayout.h"
#include "ui/LabelCustomizationPanel.h"
#include "ui/PanelAnchor.h"
#include "ui/QuickAccessService.h"

#include <cstring>
#include <imgui.h>

namespace rc {
namespace OptionsPanel {

namespace {

void RealignAnchoredPanels(AppState& state) {
    if (!state.staticDataReady) return;
    std::lock_guard lock(state.dataMutex);
    const auto raidSize =
        GridLayout::ComputePlacement(
            EncounterVisibilityFilter::FilterRaidGroups(state.raidGroups, state.raidVisibility),
            state.settings.panelLayout, state.settings.panelScale)
            .contentSize;
    const auto strikeSize =
        GridLayout::ComputePlacement(
            EncounterVisibilityFilter::FilterStrikeGroups(state.strikeGroups,
                                                          state.strikeVisibility),
            state.settings.panelLayout, state.settings.panelScale)
            .contentSize;
    if (state.settings.anchorStrikesToRaidPanel) {
        PanelAnchor::AlignStrikesToRaid(state.settings, raidSize);
    }
    if (state.settings.anchorFractalsToStrikesPanel) {
        PanelAnchor::AlignFractalsToStrikes(state.settings, strikeSize);
    }
}

}  // namespace

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

    ImGui::Checkbox("Show Raid Panel", &state.settings.raidPanel.visible);
    ImGui::Checkbox("Show Raid Encounters Panel", &state.settings.strikesPanel.visible);
    ImGui::Checkbox("Show Fractals Panel", &state.settings.fractalsPanel.visible);
    ImGui::Checkbox("Show Dungeons Panel", &state.settings.dungeonsPanel.visible);

    const char* layoutLabels[] = {"Vertical", "Horizontal"};
    int layoutIndex = state.settings.panelLayout == PanelLayout::Horizontal ? 1 : 0;
    if (ImGui::Combo("Panel layout", &layoutIndex, layoutLabels, 2)) {
        state.settings.panelLayout =
            layoutIndex == 1 ? PanelLayout::Horizontal : PanelLayout::Vertical;
        if (state.settings.anchorStrikesToRaidPanel || state.settings.anchorFractalsToStrikesPanel) {
            RealignAnchoredPanels(state);
        }
    }

    if (ImGui::Checkbox("Anchor raid encounters panel to raid panel",
                        &state.settings.anchorStrikesToRaidPanel)) {
        if (state.settings.anchorStrikesToRaidPanel) {
            RealignAnchoredPanels(state);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Keeps the raid encounters panel attached to the raid panel. Moving either panel "
            "moves both.");
    }

    if (ImGui::Checkbox("Anchor fractals panel to raid encounters panel",
                        &state.settings.anchorFractalsToStrikesPanel)) {
        if (state.settings.anchorFractalsToStrikesPanel) {
            RealignAnchoredPanels(state);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Keeps the fractals panel attached to the raid encounters panel. Moving either panel "
            "moves both, and chained raid anchoring is preserved when enabled.");
    }

    ImGui::Text("Panel visibility shortcut (NRC_TOGGLE_PANELS)");
    ImGui::Checkbox("Toggle raid panel on keybind / corner icon click",
                    &state.settings.keybindToggleRaids);
    ImGui::Checkbox("Toggle raid encounters on keybind / corner icon click",
                    &state.settings.keybindToggleStrikes);
    ImGui::Checkbox("Toggle fractals on keybind / corner icon click",
                    &state.settings.keybindToggleFractals);
    ImGui::Checkbox("Toggle dungeons on keybind / corner icon click",
                    &state.settings.keybindToggleDungeons);

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
            "Uncleared raid wing and raid encounter bosses that will not be daily bounties for "
            "the rest of "
            "the week are highlighted in orange.");
    }
    if (ImGui::Checkbox("Omit event encounters from highlight", &state.settings.omitEventEncounters)) {
        std::lock_guard lock(state.dataMutex);
        state.ApplyNonWeeklyHighlights();
    }

    ImGui::Checkbox("GW2 style background boxes", &state.settings.organicGridBoxBackgrounds);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "On: cells use organic mask textures similar to GW2 UI.\n"
            "Off: cells use plain rectangles.");
    }

    ImGui::Checkbox("Lock panel position", &state.settings.lockPanelPosition);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Prevents dragging raid and raid encounter panels. While locked, mouse input passes "
            "through "
            "the panels to the game. Tooltips still appear when hovering encounter cells.");
    }

    ImGui::Checkbox("Enable mouse hover tooltips", &state.settings.enableTooltips);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Shows enhanced encounter tooltips with boss icons and mechanic indicators.");
    }

    ImGui::Checkbox("Show raid mentor progress in tooltips", &state.settings.showMentorProgress);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Shows your raid mentor achievement progress in raid boss tooltips. Requires a GW2 "
            "API key with account and progression permissions.");
    }

    float panelScale = state.settings.panelScale;
    if (ImGui::SliderFloat("Panel scale", &panelScale, 0.5f, 2.0f, "%.2f")) {
        state.settings.panelScale = panelScale;
        if (state.settings.anchorStrikesToRaidPanel || state.settings.anchorFractalsToStrikesPanel) {
            RealignAnchoredPanels(state);
        }
    }

    float labelOpacity = state.settings.labelOpacity;
    if (ImGui::SliderFloat("Label opacity", &labelOpacity, 0.1f, 1.0f, "%.2f")) {
        state.settings.labelOpacity = labelOpacity;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Opacity for wing and group label cells.");
    }

    float gridOpacity = state.settings.gridOpacity;
    if (ImGui::SliderFloat("Grid opacity", &gridOpacity, 0.1f, 1.0f, "%.2f")) {
        state.settings.gridOpacity = gridOpacity;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Opacity for encounter cells.");
    }

    float panelBackgroundOpacity = state.settings.panelBackgroundOpacity;
    if (ImGui::SliderFloat("Grid background opacity", &panelBackgroundOpacity, 0.0f, 1.0f,
                           "%.2f")) {
        state.settings.panelBackgroundOpacity = panelBackgroundOpacity;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Opacity for the panel background behind the grid.");
    }

    ImGui::Checkbox("Highlight Emboldened raid wing", &state.settings.highlightEmbolden);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Colors wing and encounter labels for the weekly Emboldened raid wing.");
    }
    ImGui::Checkbox("Highlight Call of the Mists raid wing", &state.settings.highlightCotm);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Colors wing and encounter labels for the weekly Call of the Mists raid wing.");
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
    float textColor[3] = {state.settings.colorText.r / 255.0f,
                          state.settings.colorText.g / 255.0f,
                          state.settings.colorText.b / 255.0f};

    if (ImGui::ColorEdit3("Label text color", textColor)) {
        state.settings.colorText = {static_cast<uint8_t>(textColor[0] * 255),
                                    static_cast<uint8_t>(textColor[1] * 255),
                                    static_cast<uint8_t>(textColor[2] * 255)};
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Default color for wing and encounter label text.");
    }

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
    float emboldenColor[3] = {state.settings.colorEmbolden.r / 255.0f,
                              state.settings.colorEmbolden.g / 255.0f,
                              state.settings.colorEmbolden.b / 255.0f};
    if (ImGui::ColorEdit3("Emboldened text color", emboldenColor)) {
        state.settings.colorEmbolden = {
            static_cast<uint8_t>(emboldenColor[0] * 255),
            static_cast<uint8_t>(emboldenColor[1] * 255),
            static_cast<uint8_t>(emboldenColor[2] * 255)};
    }
    float cotmColor[3] = {state.settings.colorCotm.r / 255.0f,
                          state.settings.colorCotm.g / 255.0f,
                          state.settings.colorCotm.b / 255.0f};
    if (ImGui::ColorEdit3("Call of the Mists text color", cotmColor)) {
        state.settings.colorCotm = {static_cast<uint8_t>(cotmColor[0] * 255),
                                    static_cast<uint8_t>(cotmColor[1] * 255),
                                    static_cast<uint8_t>(cotmColor[2] * 255)};
    }

    ImGui::Separator();
    ImGui::Text("Fractals");
    if (ImGui::Checkbox("Show challenge motes", &state.settings.fractalChallengeMotes)) {
        if (state.fractalDataReady) {
            std::lock_guard lock(state.dataMutex);
            state.RebuildFractalGroups();
        }
    }
    if (ImGui::Checkbox("Show daily tier fractals", &state.settings.fractalDailyTierN)) {
        if (state.fractalDataReady) {
            std::lock_guard lock(state.dataMutex);
            state.RebuildFractalGroups();
        }
    }
    if (ImGui::Checkbox("Show daily recommended fractals", &state.settings.fractalDailyRecs)) {
        if (state.fractalDataReady) {
            std::lock_guard lock(state.dataMutex);
            state.RebuildFractalGroups();
        }
    }
    if (ImGui::Checkbox("Show tomorrow tier fractals", &state.settings.fractalTomorrowTierN)) {
        if (state.fractalDataReady) {
            std::lock_guard lock(state.dataMutex);
            state.RebuildFractalGroups();
        }
    }

    ImGui::Separator();
    ImGui::Text("Dungeons");
    ImGui::Checkbox("Show frequenter summary row", &state.settings.dungeonFrequenterVisible);
    ImGui::Checkbox("Highlight frequented paths", &state.settings.dungeonHighlightFrequenter);
    static const char* kDungeonNames[] = {"AC", "CM", "TA", "SE", "CoF", "HW", "CoE", "Arah"};
    for (size_t i = 0; i < state.settings.dungeonVisible.size(); ++i) {
        ImGui::Checkbox(kDungeonNames[i], &state.settings.dungeonVisible[i]);
    }
    float freqColor[3] = {state.settings.colorDungeonFrequenter.r / 255.0f,
                          state.settings.colorDungeonFrequenter.g / 255.0f,
                          state.settings.colorDungeonFrequenter.b / 255.0f};
    if (ImGui::ColorEdit3("Frequenter text color", freqColor)) {
        state.settings.colorDungeonFrequenter = {
            static_cast<uint8_t>(freqColor[0] * 255),
            static_cast<uint8_t>(freqColor[1] * 255),
            static_cast<uint8_t>(freqColor[2] * 255)};
        std::lock_guard lock(state.dataMutex);
        state.ApplyDungeonClears();
    }

    if (ImGui::Button("Refresh Now")) {
        state.RequestApiRefresh();
    }

    ImGui::Separator();
    EncounterSelectionPanel::Render(state);
    LabelCustomizationPanel::Render(state);

    if (!state.accountName.empty()) {
        ImGui::Text("Account: %s", state.accountName.c_str());
    }
}

}  // namespace OptionsPanel
}  // namespace rc
