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

    ColorRGB colorCleared{0, 255, 0};
    ColorRGB colorNotCleared{200, 200, 200};
    ColorRGB colorUnknown{64, 64, 64};

    void Load(const std::string& path);
    void Save(const std::string& path) const;
};

}  // namespace rc
