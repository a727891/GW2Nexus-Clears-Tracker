#include "core/SettingsStore.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace rc {

namespace {

nlohmann::json WindowToJson(const WindowState& w) {
    return {{"visible", w.visible}, {"posX", w.posX}, {"posY", w.posY}};
}

WindowState WindowFromJson(const nlohmann::json& j, const WindowState& defaults) {
    WindowState w = defaults;
    if (j.contains("visible")) w.visible = j["visible"].get<bool>();
    if (j.contains("posX")) w.posX = j["posX"].get<float>();
    if (j.contains("posY")) w.posY = j["posY"].get<float>();
    return w;
}

nlohmann::json ColorToJson(const ColorRGB& c) {
    char buf[8];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
    return buf;
}

ColorRGB ColorFromJson(const nlohmann::json& j, const ColorRGB& defaults) {
    if (j.is_string()) {
        return ColorRGB::FromHex(j.get<std::string>());
    }
    return defaults;
}

float ClampOpacity(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

}  // namespace

void SettingsStore::Load(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return;

    nlohmann::json j;
    in >> j;

    if (j.contains("apiKey")) legacyApiKey_ = j["apiKey"].get<std::string>();
    if (j.contains("pollIntervalMinutes")) pollIntervalMinutes = j["pollIntervalMinutes"].get<int>();
    if (j.contains("raidPanel")) raidPanel = WindowFromJson(j["raidPanel"], raidPanel);
    if (j.contains("strikesPanel")) strikesPanel = WindowFromJson(j["strikesPanel"], strikesPanel);
    if (j.contains("fractalsPanel")) {
        fractalsPanel = WindowFromJson(j["fractalsPanel"], fractalsPanel);
    }
    if (j.contains("dungeonsPanel")) {
        dungeonsPanel = WindowFromJson(j["dungeonsPanel"], dungeonsPanel);
    }
    if (j.contains("panelLayout")) {
        const auto value = j["panelLayout"].get<std::string>();
        if (value == "Horizontal") {
            panelLayout = PanelLayout::Horizontal;
        } else {
            panelLayout = PanelLayout::Vertical;
        }
    }
    if (j.contains("keybindToggleRaids")) {
        keybindToggleRaids = j["keybindToggleRaids"].get<bool>();
    }
    if (j.contains("keybindToggleStrikes")) {
        keybindToggleStrikes = j["keybindToggleStrikes"].get<bool>();
    }
    if (j.contains("keybindToggleFractals")) {
        keybindToggleFractals = j["keybindToggleFractals"].get<bool>();
    }
    if (j.contains("keybindToggleDungeons")) {
        keybindToggleDungeons = j["keybindToggleDungeons"].get<bool>();
    }
    if (!j.contains("keybindToggleRaids") && !j.contains("keybindToggleStrikes") &&
        j.contains("keybindTogglesBothPanels") && j["keybindTogglesBothPanels"].get<bool>()) {
        keybindToggleRaids = true;
        keybindToggleStrikes = true;
    }
    if (j.contains("cornerIconEnabled")) cornerIconEnabled = j["cornerIconEnabled"].get<bool>();
    if (j.contains("highlightNonWeeklyBounty")) {
        highlightNonWeeklyBounty = j["highlightNonWeeklyBounty"].get<bool>();
    }
    if (j.contains("omitEventEncounters")) omitEventEncounters = j["omitEventEncounters"].get<bool>();
    if (j.contains("anchorStrikesToRaidPanel")) {
        anchorStrikesToRaidPanel = j["anchorStrikesToRaidPanel"].get<bool>();
    }
    if (j.contains("anchorFractalsToStrikesPanel")) {
        anchorFractalsToStrikesPanel = j["anchorFractalsToStrikesPanel"].get<bool>();
    }
    if (j.contains("organicGridBoxBackgrounds")) {
        organicGridBoxBackgrounds = j["organicGridBoxBackgrounds"].get<bool>();
    }
    if (j.contains("lockPanelPosition")) {
        lockPanelPosition = j["lockPanelPosition"].get<bool>();
    }
    if (j.contains("enableTooltips")) {
        enableTooltips = j["enableTooltips"].get<bool>();
    }
    if (j.contains("showMentorProgress")) {
        showMentorProgress = j["showMentorProgress"].get<bool>();
    }
    if (j.contains("lastShownMotdId")) {
        lastShownMotdId = j["lastShownMotdId"].get<std::string>();
    }
    if (j.contains("fractalChallengeMotes")) {
        fractalChallengeMotes = j["fractalChallengeMotes"].get<bool>();
    }
    if (j.contains("fractalDailyTierN")) {
        fractalDailyTierN = j["fractalDailyTierN"].get<bool>();
    }
    if (j.contains("fractalDailyRecs")) {
        fractalDailyRecs = j["fractalDailyRecs"].get<bool>();
    }
    if (j.contains("fractalTomorrowTierN")) {
        fractalTomorrowTierN = j["fractalTomorrowTierN"].get<bool>();
    }
    if (j.contains("dungeonFrequenterVisible")) {
        dungeonFrequenterVisible = j["dungeonFrequenterVisible"].get<bool>();
    }
    if (j.contains("dungeonHighlightFrequenter")) {
        dungeonHighlightFrequenter = j["dungeonHighlightFrequenter"].get<bool>();
    }
    if (j.contains("dungeonVisible") && j["dungeonVisible"].is_array()) {
        for (size_t i = 0; i < dungeonVisible.size() && i < j["dungeonVisible"].size(); ++i) {
            dungeonVisible[i] = j["dungeonVisible"][i].get<bool>();
        }
    }
    if (j.contains("panelScale")) panelScale = j["panelScale"].get<float>();
    if (j.contains("labelOpacity")) {
        labelOpacity = ClampOpacity(j["labelOpacity"].get<float>(), 0.1f, 1.0f);
    }
    if (j.contains("gridOpacity")) {
        gridOpacity = ClampOpacity(j["gridOpacity"].get<float>(), 0.1f, 1.0f);
    }
    if (j.contains("panelBackgroundOpacity")) {
        panelBackgroundOpacity = ClampOpacity(j["panelBackgroundOpacity"].get<float>(), 0.0f, 1.0f);
    }
    if (j.contains("highlightEmbolden")) highlightEmbolden = j["highlightEmbolden"].get<bool>();
    if (j.contains("highlightCotm")) highlightCotm = j["highlightCotm"].get<bool>();
    if (j.contains("colorText")) colorText = ColorFromJson(j["colorText"], colorText);
    if (j.contains("colorCleared")) colorCleared = ColorFromJson(j["colorCleared"], colorCleared);
    if (j.contains("colorNotCleared")) colorNotCleared = ColorFromJson(j["colorNotCleared"], colorNotCleared);
    if (j.contains("colorUnknown")) colorUnknown = ColorFromJson(j["colorUnknown"], colorUnknown);
    if (j.contains("colorNonWeeklyBounty")) {
        colorNonWeeklyBounty = ColorFromJson(j["colorNonWeeklyBounty"], colorNonWeeklyBounty);
    }
    if (j.contains("colorEmbolden")) colorEmbolden = ColorFromJson(j["colorEmbolden"], colorEmbolden);
    if (j.contains("colorCotm")) colorCotm = ColorFromJson(j["colorCotm"], colorCotm);
    if (j.contains("colorDungeonFrequenter")) {
        colorDungeonFrequenter = ColorFromJson(j["colorDungeonFrequenter"], colorDungeonFrequenter);
    }
}

void SettingsStore::Save(const std::string& path) const {
    nlohmann::json j = {
        {"pollIntervalMinutes", pollIntervalMinutes},
        {"raidPanel", WindowToJson(raidPanel)},
        {"strikesPanel", WindowToJson(strikesPanel)},
        {"fractalsPanel", WindowToJson(fractalsPanel)},
        {"dungeonsPanel", WindowToJson(dungeonsPanel)},
        {"panelLayout", panelLayout == PanelLayout::Horizontal ? "Horizontal" : "Vertical"},
        {"keybindToggleRaids", keybindToggleRaids},
        {"keybindToggleStrikes", keybindToggleStrikes},
        {"keybindToggleFractals", keybindToggleFractals},
        {"keybindToggleDungeons", keybindToggleDungeons},
        {"cornerIconEnabled", cornerIconEnabled},
        {"highlightNonWeeklyBounty", highlightNonWeeklyBounty},
        {"omitEventEncounters", omitEventEncounters},
        {"anchorStrikesToRaidPanel", anchorStrikesToRaidPanel},
        {"anchorFractalsToStrikesPanel", anchorFractalsToStrikesPanel},
        {"organicGridBoxBackgrounds", organicGridBoxBackgrounds},
        {"lockPanelPosition", lockPanelPosition},
        {"enableTooltips", enableTooltips},
        {"showMentorProgress", showMentorProgress},
        {"lastShownMotdId", lastShownMotdId},
        {"fractalChallengeMotes", fractalChallengeMotes},
        {"fractalDailyTierN", fractalDailyTierN},
        {"fractalDailyRecs", fractalDailyRecs},
        {"fractalTomorrowTierN", fractalTomorrowTierN},
        {"dungeonFrequenterVisible", dungeonFrequenterVisible},
        {"dungeonHighlightFrequenter", dungeonHighlightFrequenter},
        {"dungeonVisible", dungeonVisible},
        {"panelScale", panelScale},
        {"labelOpacity", labelOpacity},
        {"gridOpacity", gridOpacity},
        {"panelBackgroundOpacity", panelBackgroundOpacity},
        {"highlightEmbolden", highlightEmbolden},
        {"highlightCotm", highlightCotm},
        {"colorText", ColorToJson(colorText)},
        {"colorCleared", ColorToJson(colorCleared)},
        {"colorNotCleared", ColorToJson(colorNotCleared)},
        {"colorUnknown", ColorToJson(colorUnknown)},
        {"colorNonWeeklyBounty", ColorToJson(colorNonWeeklyBounty)},
        {"colorEmbolden", ColorToJson(colorEmbolden)},
        {"colorCotm", ColorToJson(colorCotm)},
        {"colorDungeonFrequenter", ColorToJson(colorDungeonFrequenter)},
    };

    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    std::ofstream out(path);
    out << j.dump(2);
}

std::string SettingsStore::ConsumeLegacyApiKey() {
    std::string key = std::move(legacyApiKey_);
    legacyApiKey_.clear();
    return key;
}

}  // namespace rc
