#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "core/Branding.h"
#include "ui/OptionsCommon.h"
#include "ui/OptionsPanelAppearance.h"
#include "ui/OptionsUiKit.h"
#include "ui/QuickAccessService.h"

#include <cstring>
#include <imgui.h>
#include <mutex>

#ifdef _WIN32
#include <shellapi.h>
#endif

namespace rc {
namespace OptionsGeneralTab {
namespace {

#ifdef _WIN32
void OpenExternalUrl(const char* url) {
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}
#else
void OpenExternalUrl(const char* url) { (void)url; }
#endif

enum class Section { ApiSync = 0, CornerIcon, GlobalOptions, About };

void RenderApiSync(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("API & Sync");
    SectionSubtext("Register GW2 API keys for clear tracking.");

    static char apiKeyBuf[128] = {};
    const bool registering = state.accountRegistry.IsRegistering();

    ImGui::InputText("GW2 API Key", apiKeyBuf, sizeof(apiKeyBuf), ImGuiInputTextFlags_Password);
    if (!registering && ImGui::Button("Register Key")) {
        state.RegisterApiKey(apiKeyBuf);
        apiKeyBuf[0] = '\0';
    }
    if (ImGui::IsItemHovered()) {
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

    const auto accounts = state.accountRegistry.AccountsSnapshot();
    const auto activeTokenId = state.accountRegistry.ActiveTokenId();

    if (accounts.empty()) {
        ImGui::TextDisabled("No API keys registered.");
    } else if (ImGui::BeginTable("api_accounts", 4,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Key");
        ImGui::TableSetupColumn("Account");
        ImGui::TableSetupColumn("Chars");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        for (const auto& account : accounts) {
            ImGui::TableNextRow();
            ImGui::PushID(account.tokenId.c_str());

            ImGui::TableNextColumn();
            std::string keyLabel = account.keyName;
            if (activeTokenId && *activeTokenId == account.tokenId) {
                keyLabel += " (active)";
            }
            ImGui::TextUnformatted(keyLabel.c_str());

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(account.accountName.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%zu", account.characters.size());

            ImGui::TableNextColumn();
            if (ImGui::Button("Remove")) {
                state.RemoveApiKey(account.tokenId);
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    if (!state.characterName.empty()) {
        ImGui::Text("Character: %s", state.characterName.c_str());
    } else {
        ImGui::TextDisabled("Character: (not in game)");
    }

    if (!state.accountName.empty()) {
        ImGui::Text("Active account: %s", state.accountName.c_str());
    } else if (!state.characterName.empty()) {
        WarningText(
            "No registered key matches the current character. API clears unavailable.");
    } else {
        ImGui::TextDisabled("Active account: (none)");
    }

    ImGui::Spacing();
    int pollMinutes = state.settings.pollIntervalMinutes;
    if (SettingSliderInt("API poll interval (min)", &pollMinutes, 1, 30,
                         "How often clears are refreshed from the GW2 API.")) {
        state.settings.pollIntervalMinutes = pollMinutes;
        state.apiPoll.SetIntervalMinutes(pollMinutes);
    }

    if (ImGui::Button("Refresh Now")) {
        state.RequestApiRefresh();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Immediately fetch clears from the GW2 API for the active account.");
    }
}

void RenderCornerIcon(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Corner Icon");
    SectionSubtext("Quick-access corner icon shortcut (ALT+SHIFT+R).");

    if (SettingCheckbox("Show corner icon", &state.settings.cornerIconEnabled,
                        "Shows the quick-access corner icon for panel toggles and MOTD.")) {
        if (state.api) {
            QuickAccessService::SyncVisibility(state.api, state);
        }
    }

    ImGui::Spacing();
    SectionSubtext("Panel visibility shortcut (CLRTRK_TOGGLE_PANELS)");
    SettingCheckbox("Toggle raid panel on keybind / corner icon click",
                    &state.settings.keybindToggleRaids);
    SettingCheckbox("Toggle raid encounters on keybind / corner icon click",
                    &state.settings.keybindToggleStrikes);
    SettingCheckbox("Toggle fractals on keybind / corner icon click",
                    &state.settings.keybindToggleFractals);
    SettingCheckbox("Toggle dungeons on keybind / corner icon click",
                    &state.settings.keybindToggleDungeons);
}

void RenderGlobalOptions(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Global Options");

    SettingCheckbox("Clamp panels to screen", &state.settings.screenClamp,
                    "Keeps overlay panels within the game window when dragged.");

    SettingCheckbox("GW2 style background boxes", &state.settings.organicGridBoxBackgrounds,
                    "Cell backgrounds styled like GW2 watercolor brush strokes.");

    if (SettingCheckbox("Enable mouse hover tooltips", &state.settings.globalEnableTooltips,
                        "Shows enhanced encounter tooltips with boss icons and mechanic "
                        "indicators. Overwrites per-panel tooltip settings.")) {
        state.settings.ApplyGlobalTooltipsToAllPanels();
    }

    ImGui::Spacing();
    SectionHeading("Global Appearance");
    WarningText(
        "Changes here overwrite panel style on Raids, Strikes, Fractals, and Dungeons, including "
        "any per-panel overrides.");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(GrayColor(),
                       "Use each panel's Options and Style tab to customize a single panel.");
    ImGui::PopTextWrapPos();

    if (OptionsPanelAppearance::RenderFields(state.settings.globalAppearance, state.settings,
                                             true)) {
        state.settings.ApplyGlobalAppearanceToAllPanels();
        if (state.settings.anchorStrikesToRaidPanel ||
            state.settings.anchorFractalsToStrikesPanel) {
            RealignAnchoredPanels(state);
        }
    }

    ImGui::Spacing();
    SectionHeading("Clear State Colors");
    SettingColorRgb("Label text color", state.settings.colorText,
                    "Default color for wing and encounter label text.");
    SettingColorRgb("Cleared color", state.settings.colorCleared);
    SettingColorRgb("Not cleared color", state.settings.colorNotCleared);
    SettingColorRgb("Unknown color", state.settings.colorUnknown);
}

void RenderAbout() {
    using namespace OptionsUiKit;

    SectionHeading("About");
    ImGui::TextWrapped(
        "%s is a Nexus port of the Blish HUD Clears Tracker module. "
        "It tracks your daily and weekly PvE instance clears for raids, raid encounters, "
        "fractals, and dungeons using the Guild Wars 2 API.",
        kDisplayName);

    ImGui::Spacing();
    if (ImGui::Button("Patch Notes")) {
        OpenExternalUrl(kPatchNotesUrl);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Open patch notes in your default web browser.");
    }

    SectionHeading("Credits");
    SectionSubtext("Developers");
    ImGui::BulletText("Soeed");

    ImGui::Spacing();
    SectionSubtext("Thank you to:");
    ImGui::BulletText(
        "Freesnöw (BlishHUD) for continued static data and config file hosting");
    ImGui::BulletText("Invisi for challenge mote instabilities information");
    // ImGui::BulletText("Naru for French translations in the original Blish module");

    ImGui::Spacing();
    SectionSubtext("Inspiration");
    ImGui::BulletText("Gw2TaCO - raid tracking feature");

    SectionHeading("Important URLs");
    ImGui::TextWrapped("Original module:");
    ImGui::TextWrapped("https://github.com/a727891/BlishHud-Raid-Clears");
    ImGui::Spacing();
    // ImGui::TextWrapped("Static encounter data:");
    // ImGui::TextWrapped("https://bhm.blishhud.com/Soeed.RaidClears/static/v2/");
    // ImGui::Spacing();
    ImGui::TextWrapped("Challenge mote instabilities:");
    ImGui::TextWrapped("https://github.com/Invisi/gw2-fotm-instabilities");
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"API & Sync", "Corner Icon", "Global Options", "About"};

    BeginTabPage(section, kSections, 4);
    ImGui::PushID("general");
    BeginContentPanel(nullptr);

    switch (static_cast<Section>(section)) {
        case Section::ApiSync:
            RenderApiSync(state);
            break;
        case Section::CornerIcon:
            RenderCornerIcon(state);
            break;
        case Section::GlobalOptions:
            RenderGlobalOptions(state);
            break;
        case Section::About:
            RenderAbout();
            break;
    }

    EndContentPanel();
    ImGui::PopID();
    EndTabPage();
}

}  // namespace OptionsGeneralTab
}  // namespace rc
