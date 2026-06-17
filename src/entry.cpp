#include "core/AppState.h"
#include "core/Branding.h"
#include "Version.h"
#include "core/MumbleIdentity.h"
#include "core/PanelVisibility.h"
#include "ui/QuickAccessService.h"

#include <imgui.h>
#include "mumble/Mumble.h"
#include "nexus/Nexus.h"
#include "ui/SettingsWindow.h"
#include "ui/MentorProgressPopupService.h"
#include "ui/RaidPanel.h"
#include <windows.h>

namespace {

AddonAPI_t* g_api = nullptr;
float g_lastFrameTime = 0.0f;

constexpr const char* kTogglePanels = "CLRTRK_TOGGLE_PANELS";

void AddonLoad(AddonAPI_t* api);
void AddonUnload();
void AddonRender();
void AddonOptions();

void OnTogglePanels(const char*, bool isRelease) {
    if (isRelease) return;
    auto& state = rc::AppState::Instance();
    rc::QuickAccessService::OnShortcutActivated(state);
    TogglePanelsViaShortcut(state.settings);
}

void AddonLoad(AddonAPI_t* api) {
    g_api = api;
    api->Log(LOGL_INFO, "NexusRaidClears", "AddonLoad starting.");

    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(api->ImguiContext));
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<void* (*)(size_t, void*)>(api->ImguiMalloc),
        reinterpret_cast<void (*)(void*, void*)>(api->ImguiFree));

    auto& state = rc::AppState::Instance();
    state.Initialize(api);

    api->GUI_Register(RT_Render, AddonRender);
    api->GUI_Register(RT_OptionsRender, AddonOptions);

    api->InputBinds_RegisterWithString(kTogglePanels, OnTogglePanels, "ALT+SHIFT+R");

    api->Log(LOGL_INFO, "NexusRaidClears", "Loaded.");
}

void AddonUnload() {
    if (!g_api) return;
    g_api->GUI_Deregister(AddonRender);
    g_api->GUI_Deregister(AddonOptions);
    g_api->InputBinds_Deregister(kTogglePanels);
    rc::QuickAccessService::Unregister(g_api);
    rc::AppState::Instance().Shutdown();
    g_api->Log(LOGL_INFO, "NexusRaidClears", "Unloaded.");
    g_api = nullptr;
}

void AddonRender() {
    auto& state = rc::AppState::Instance();
    const float now = static_cast<float>(ImGui::GetTime());
    const float delta = g_lastFrameTime > 0.0f ? now - g_lastFrameTime : 0.0f;
    g_lastFrameTime = now;

    state.TickResets();
    state.apiPoll.Update(delta);
    state.ProcessPendingStaticDataLoad();
    state.ProcessPendingUiAssetsInit();
    state.ProcessPendingApiRefresh();

    if (state.pendingAccountChanged.exchange(false)) {
        state.OnActiveAccountChanged();
    } else if (state.pendingCharacterResolve.exchange(false)) {
        const auto charName = rc::MumbleIdentity::ParseIdentityName(state.mumbleLink);
        state.UpdateActiveCharacter(charName, true);
    } else if (state.pendingTooltipRefresh.exchange(false)) {
        state.accountName = state.accountRegistry.ActiveAccountName().value_or("");
        state.mentorProgress.SetActiveAccount(state.accountName);
        rc::QuickAccessService::Refresh(g_api, state);
    }

    if (state.mumbleLink) {
        const uint32_t mapId = Mumble::GetMapId(state.mumbleLink);
        state.mapWatcher.Update(mapId);
        state.fractalMapWatcher.Update(mapId);

        const auto charName = rc::MumbleIdentity::ParseIdentityName(state.mumbleLink);
        state.UpdateActiveCharacter(charName);
    }

    rc::RaidPanel::Render(state);
    rc::StrikesPanel::Render(state);
    rc::FractalsPanel::Render(state);
    rc::DungeonsPanel::Render(state);
    rc::MentorProgressPopupService::Render(state);

    if (rc::SettingsWindow::IsOpen()) {
        rc::SettingsWindow::Render(state);
    }
}

void AddonOptions() {
    rc::OptionsPanel::RenderNexusConfigEntry(rc::AppState::Instance());
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID) { return TRUE; }

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef() {
    static AddonDefinition_t def{};
    static bool initialized = false;
    if (!initialized) {
        def.Signature = rc::kSignature;
        def.APIVersion = NEXUS_API_VERSION;
        def.Name = rc::kDisplayName;
        def.Version = {V_MAJOR, V_MINOR, V_BUILD, V_REVISION};
        def.Author = "Soeed";
        def.Description = rc::kDescription;
        def.Load = AddonLoad;
        def.Unload = AddonUnload;
        def.Flags = AF_None;
        def.Provider = UP_None;
        initialized = true;
    }
    return &def;
}
