#pragma once

#include "core/Types.h"
#include <array>
#include <string>

namespace rc {

class SettingsStore {
public:
    int pollIntervalMinutes = 5;

    WindowState raidPanel{true, 367.0f, 165.0f};
    WindowState strikesPanel{true, 365.0f, 421.0f};
    WindowState fractalsPanel{true, 364.0f, 581.0f};
    WindowState dungeonsPanel{true, 364.0f, 675.0f};

    PanelLayout panelLayout = PanelLayout::Vertical;
    GroupLabelDisplay groupLabelDisplay = GroupLabelDisplay::Abbreviation;
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
    bool screenClamp = true;
    bool enableTooltips = true;
    bool showMentorProgress = true;
    bool showMentorProgressPopup = false;
    bool mentorProgressPopupReposition = false;
    float mentorProgressPopupPosX = -1.0f;
    float mentorProgressPopupPosY = -1.0f;
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
