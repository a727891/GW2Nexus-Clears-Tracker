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
}

}  // namespace rc
