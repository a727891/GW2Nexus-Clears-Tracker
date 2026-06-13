#pragma once

#include "core/Types.h"
#include <string>

namespace rc {

class SettingsStore {
public:
    std::string apiKey;
    int pollIntervalMinutes = 5;

    WindowState raidPanel;
    WindowState strikesPanel;

    PanelLayout panelLayout = PanelLayout::Vertical;
    float panelScale = 1.0f;

    float labelOpacity = 1.0f;
    float gridOpacity = 0.8f;
    float panelBackgroundOpacity = 0.0f;

    bool keybindToggleRaids = true;
    bool keybindToggleStrikes = true;

    bool cornerIconEnabled = true;
    bool highlightNonWeeklyBounty = true;
    bool omitEventEncounters = true;
    bool anchorStrikesToRaidPanel = false;
    bool organicGridBoxBackgrounds = true;
    bool lockPanelPosition = false;

    bool highlightEmbolden = true;
    bool highlightCotm = true;

    ColorRGB colorText{255, 255, 255};
    ColorRGB colorCleared{0, 255, 0};
    ColorRGB colorNotCleared{200, 200, 200};
    ColorRGB colorUnknown{64, 64, 64};
    ColorRGB colorNonWeeklyBounty{204, 136, 0};
    ColorRGB colorEmbolden{0, 0, 255};
    ColorRGB colorCotm{255, 255, 0};

    void Load(const std::string& path);
    void Save(const std::string& path) const;
};

}  // namespace rc
