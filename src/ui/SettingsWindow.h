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

void Open(SettingsTab tab = SettingsTab::General);
void Close();
void Toggle(SettingsTab tab = SettingsTab::General);
bool IsOpen();
void Shutdown(AddonAPI_t* api);
void Render(AppState& state);

}  // namespace SettingsWindow

namespace OptionsPanel {

void RenderNexusConfigEntry(AppState& state);
void RenderWindow(AppState& state, SettingsTab pendingTab, bool applyPendingTab);

}  // namespace OptionsPanel
}  // namespace rc
