#include "core/AppState.h"
#include "ui/RaidPanel.h"

#include "imgui.h"
#include "mumble/Mumble.h"
#include "nexus/Nexus.h"

#include <windows.h>

namespace {

AddonAPI_t* g_api = nullptr;
float g_lastFrameTime = 0.0f;

void AddonLoad(AddonAPI_t* api);
void AddonUnload();
void AddonRender();
void AddonOptions();

void OnToggleRaids(const char*, bool) {
    rc::AppState::Instance().settings.raidPanel.visible =
        !rc::AppState::Instance().settings.raidPanel.visible;
}

void OnToggleStrikes(const char*, bool) {
    rc::AppState::Instance().settings.strikesPanel.visible =
        !rc::AppState::Instance().settings.strikesPanel.visible;
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
    api->GUI_RegisterCloseOnEscape("Raid Clears", &state.settings.raidPanel.visible);
    api->GUI_RegisterCloseOnEscape("Strike Clears", &state.settings.strikesPanel.visible);

    api->InputBinds_RegisterWithString("NRC_TOGGLE_RAIDS", OnToggleRaids, "ALT+SHIFT+R");
    api->InputBinds_RegisterWithString("NRC_TOGGLE_STRIKES", OnToggleStrikes, "ALT+SHIFT+S");

    api->Log(LOGL_INFO, "NexusRaidClears", "Loaded.");
}

void AddonUnload() {
    if (!g_api) return;
    g_api->GUI_Deregister(AddonRender);
    g_api->GUI_Deregister(AddonOptions);
    g_api->InputBinds_Deregister("NRC_TOGGLE_RAIDS");
    g_api->InputBinds_Deregister("NRC_TOGGLE_STRIKES");
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
    state.ProcessPendingApiRefresh();

    if (state.mumbleLink) {
        const uint32_t mapId = Mumble::GetMapId(state.mumbleLink);
        state.mapWatcher.Update(mapId);
    }

    rc::RaidPanel::Render(state);
    rc::StrikesPanel::Render(state);
}

void AddonOptions() { rc::OptionsPanel::Render(rc::AppState::Instance()); }

}  // namespace

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID) { return TRUE; }

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef() {
    static AddonDefinition_t def{};
    static bool initialized = false;
    if (!initialized) {
        def.Signature = -2024061201;
        def.APIVersion = NEXUS_API_VERSION;
        def.Name = "Nexus Raid Clears";
        def.Version = {1, 0, 0, 0};
        def.Author = "Soeed";
        def.Description = "Raid and strike clear overlay panels for Guild Wars 2.";
        def.Load = AddonLoad;
        def.Unload = AddonUnload;
        def.Flags = AF_None;
        def.Provider = UP_None;
        initialized = true;
    }
    return &def;
}
