#include "ui/QuickAccessService.h"

#include "core/AppState.h"

#include <filesystem>
#include <imgui.h>
#include <string>

namespace rc {
namespace QuickAccessService {
namespace {

constexpr const char* kShortcutId = "NRC_QUICKACCESS";
constexpr const char* kTogglePanelsBind = "NRC_TOGGLE_PANELS";
constexpr const char* kIconTex = "NRC_ICON";
constexpr const char* kIconHoverTex = "NRC_ICON_HOVER";
constexpr const char* kContextMenuId = "NRC_CTX_MENU";

bool registered_ = false;
std::string cachedTooltip_;

void RenderContextMenu() {
    auto& settings = AppState::Instance().settings;

    const char* raidsLabel = settings.raidPanel.visible ? "Hide Raid Panel" : "Show Raid Panel";
    if (ImGui::Selectable(raidsLabel)) {
        settings.raidPanel.visible = !settings.raidPanel.visible;
    }

    const char* strikesLabel =
        settings.strikesPanel.visible ? "Hide Raid Encounters" : "Show Raid Encounters";
    if (ImGui::Selectable(strikesLabel)) {
        settings.strikesPanel.visible = !settings.strikesPanel.visible;
    }

    if (ImGui::Selectable("Refresh API")) {
        AppState::Instance().RequestApiRefresh();
    }

    ImGui::Separator();
    const char* fractalsLabel =
        settings.fractalsPanel.visible ? "Hide Fractals" : "Show Fractals";
    if (ImGui::Selectable(fractalsLabel)) {
        settings.fractalsPanel.visible = !settings.fractalsPanel.visible;
    }

    const char* dungeonsLabel =
        settings.dungeonsPanel.visible ? "Hide Dungeons" : "Show Dungeons";
    if (ImGui::Selectable(dungeonsLabel)) {
        settings.dungeonsPanel.visible = !settings.dungeonsPanel.visible;
    }
}

void LoadTextures(AddonAPI_t* api, const std::string& addonDir) {
    const auto iconPath = (std::filesystem::path(addonDir) / "textures" / "raidIconDark.png").string();
    const auto hoverPath =
        (std::filesystem::path(addonDir) / "textures" / "raidIconBright.png").string();
    api->Textures_GetOrCreateFromFile(kIconTex, iconPath.c_str());
    api->Textures_GetOrCreateFromFile(kIconHoverTex, hoverPath.c_str());
}

std::string BuildTooltip(const AppState& state) {
    std::string tooltip = "Nexus Raid Clears";
    if (!state.characterName.empty()) {
        tooltip += "\nCharacter: " + state.characterName;
    }
    if (!state.accountName.empty()) {
        tooltip += "\nAccount: " + state.accountName;
    } else if (!state.characterName.empty()) {
        tooltip += "\nNo registered key matches this character.";
    }
    if (!state.motd.empty()) {
        tooltip += "\n\n" + state.motd;
    }
    return tooltip;
}

bool HasUnreadMotd(const AppState& state) {
    return !state.motd.empty() && !state.motdId.empty() &&
           state.settings.lastShownMotdId != state.motdId;
}

void UpdateMotdNotification(AddonAPI_t* api, const AppState& state) {
    if (!api || !registered_) return;
    if (HasUnreadMotd(state)) {
        api->QuickAccess_Notify(kShortcutId);
    }
}

void ReregisterShortcut(AddonAPI_t* api, AppState& state) {
    if (!api || !registered_) return;

    api->QuickAccess_RemoveContextMenu(kContextMenuId);
    api->QuickAccess_Remove(kShortcutId);
    registered_ = false;
    Register(api, state);
}

}  // namespace

void Register(AddonAPI_t* api, AppState& state) {
    if (!api || registered_) return;

    LoadTextures(api, state.addonDir);

    cachedTooltip_ = BuildTooltip(state);
    api->QuickAccess_Add(kShortcutId, kIconTex, kIconHoverTex, kTogglePanelsBind,
                         cachedTooltip_.c_str());
    api->QuickAccess_AddContextMenu(kContextMenuId, kShortcutId, RenderContextMenu);
    registered_ = true;
    UpdateMotdNotification(api, state);
}

void Unregister(AddonAPI_t* api) {
    if (!api || !registered_) return;

    api->QuickAccess_RemoveContextMenu(kContextMenuId);
    api->QuickAccess_Remove(kShortcutId);
    registered_ = false;
    cachedTooltip_.clear();
}

void Refresh(AddonAPI_t* api, AppState& state) {
    if (!api || !state.settings.cornerIconEnabled) return;

    if (!registered_) {
        Register(api, state);
        return;
    }

    const auto tooltip = BuildTooltip(state);
    if (tooltip != cachedTooltip_) {
        ReregisterShortcut(api, state);
    } else {
        UpdateMotdNotification(api, state);
    }
}

void OnShortcutActivated(AppState& state) {
    if (state.motdId.empty() || state.settings.lastShownMotdId == state.motdId) return;
    state.settings.lastShownMotdId = state.motdId;
}

void SyncVisibility(AddonAPI_t* api, AppState& state) {
    if (!api) return;
    if (state.settings.cornerIconEnabled) {
        Refresh(api, state);
    } else {
        Unregister(api);
    }
}

}  // namespace QuickAccessService
}  // namespace rc
