#pragma once

#include "core/Types.h"
#include <array>
#include <string>

namespace rc {

class SettingsStore {
public:
    int pollIntervalMinutes = 5;

    WindowState raidPanel;
    WindowState strikesPanel;
    WindowState fractalsPanel{true, 250.0f, 525.0f};
    WindowState dungeonsPanel{true, 250.0f, 605.0f};

    PanelLayout panelLayout = PanelLayout::Vertical;
    float panelScale = 1.0f;

    float labelOpacity = 1.0f;
    float gridOpacity = 0.8f;
    float panelBackgroundOpacity = 0.0f;

    bool keybindToggleRaids = true;
    bool keybindToggleStrikes = true;
    bool keybindToggleFractals = true;
    bool keybindToggleDungeons = true;

    bool cornerIconEnabled = true;
    bool highlightNonWeeklyBounty = true;
    bool omitEventEncounters = true;
    bool anchorStrikesToRaidPanel = false;
    bool anchorFractalsToStrikesPanel = false;
    bool organicGridBoxBackgrounds = true;
    bool lockPanelPosition = false;
    bool enableTooltips = true;
    bool showMentorProgress = true;
    std::string lastShownMotdId;

    bool fractalChallengeMotes = true;
    bool fractalDailyTierN = true;
    bool fractalDailyRecs = true;
    bool fractalTomorrowTierN = false;

    bool dungeonFrequenterVisible = true;
    bool dungeonHighlightFrequenter = true;
    std::array<bool, 8> dungeonVisible = {true, true, true, true, true, true, true, true};

    bool highlightEmbolden = true;
    bool highlightCotm = true;

    ColorRGB colorText{255, 255, 255};
    ColorRGB colorCleared{0, 255, 0};
    ColorRGB colorNotCleared{200, 200, 200};
    ColorRGB colorUnknown{64, 64, 64};
    ColorRGB colorNonWeeklyBounty{204, 136, 0};
    ColorRGB colorEmbolden{0, 0, 255};
    ColorRGB colorCotm{255, 255, 0};
    ColorRGB colorDungeonFrequenter{255, 255, 0};

    void Load(const std::string& path);
    void Save(const std::string& path) const;
    std::string ConsumeLegacyApiKey();

private:
    std::string legacyApiKey_;
};

}  // namespace rc
