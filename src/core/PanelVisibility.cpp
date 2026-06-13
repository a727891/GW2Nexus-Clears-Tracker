#include "core/PanelVisibility.h"

#include "core/SettingsStore.h"

namespace rc {

void TogglePanelsViaShortcut(SettingsStore& settings) {
    if (settings.keybindToggleRaids) {
        settings.raidPanel.visible = !settings.raidPanel.visible;
    }
    if (settings.keybindToggleStrikes) {
        settings.strikesPanel.visible = !settings.strikesPanel.visible;
    }
    if (settings.keybindToggleFractals) {
        settings.fractalsPanel.visible = !settings.fractalsPanel.visible;
    }
    if (settings.keybindToggleDungeons) {
        settings.dungeonsPanel.visible = !settings.dungeonsPanel.visible;
    }
}

}  // namespace rc
