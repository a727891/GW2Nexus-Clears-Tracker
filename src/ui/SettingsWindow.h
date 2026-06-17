#pragma once

#include "nexus/Nexus.h"

namespace rc {

class AppState;

enum class SettingsTab : int {
    General = 0,
    Raids = 1,
    Strikes = 2,
    Fractals = 3,
    Dungeons = 4,
};

namespace SettingsWindow {

constexpr int kNoGeneralSection = -1;
constexpr int kGeneralSectionApiSync = 0;

void Open(SettingsTab tab = SettingsTab::General, int generalSection = kNoGeneralSection);
void Close();
void Toggle(SettingsTab tab = SettingsTab::General, int generalSection = kNoGeneralSection);
bool IsOpen();
void Shutdown(AddonAPI_t* api);
void Render(AppState& state);

}  // namespace SettingsWindow

namespace OptionsPanel {

void RenderNexusConfigEntry(AppState& state);
void RenderWindow(AppState& state, SettingsTab pendingTab, bool applyPendingTab,
                  int pendingGeneralSection, bool applyPendingGeneralSection);

}  // namespace OptionsPanel
}  // namespace rc
