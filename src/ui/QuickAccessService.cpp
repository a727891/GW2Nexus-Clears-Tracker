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

void RenderContextMenu() {
    auto& settings = AppState::Instance().settings;

    const char* raidsLabel = settings.raidPanel.visible ? "Hide Raids" : "Show Raids";
    if (ImGui::Selectable(raidsLabel)) {
        settings.raidPanel.visible = !settings.raidPanel.visible;
    }

    const char* strikesLabel = settings.strikesPanel.visible ? "Hide Strikes" : "Show Strikes";
    if (ImGui::Selectable(strikesLabel)) {
        settings.strikesPanel.visible = !settings.strikesPanel.visible;
    }

    if (ImGui::Selectable("Refresh API")) {
        AppState::Instance().RequestApiRefresh();
    }
}

void LoadTextures(AddonAPI_t* api, const std::string& addonDir) {
    const auto iconPath = (std::filesystem::path(addonDir) / "textures" / "raidIconDark.png").string();
    const auto hoverPath =
        (std::filesystem::path(addonDir) / "textures" / "raidIconBright.png").string();
    api->Textures_GetOrCreateFromFile(kIconTex, iconPath.c_str());
    api->Textures_GetOrCreateFromFile(kIconHoverTex, hoverPath.c_str());
}

}  // namespace

void Register(AddonAPI_t* api, AppState& state) {
    if (!api || registered_) return;

    LoadTextures(api, state.addonDir);

    std::string tooltip = "Nexus Raid Clears";
    if (!state.accountName.empty()) {
        tooltip += "\n" + state.accountName;
    }

    api->QuickAccess_Add(kShortcutId, kIconTex, kIconHoverTex, kTogglePanelsBind, tooltip.c_str());
    api->QuickAccess_AddContextMenu(kContextMenuId, kShortcutId, RenderContextMenu);
    registered_ = true;
}

void Unregister(AddonAPI_t* api) {
    if (!api || !registered_) return;

    api->QuickAccess_RemoveContextMenu(kContextMenuId);
    api->QuickAccess_Remove(kShortcutId);
    registered_ = false;
}

void SyncVisibility(AddonAPI_t* api, AppState& state) {
    if (!api) return;
    if (state.settings.cornerIconEnabled) {
        if (!registered_) Register(api, state);
    } else {
        Unregister(api);
    }
}

}  // namespace QuickAccessService
}  // namespace rc
