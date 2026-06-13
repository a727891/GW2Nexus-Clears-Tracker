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

}  // namespace

void SettingsStore::Load(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return;

    nlohmann::json j;
    in >> j;

    if (j.contains("apiKey")) apiKey = j["apiKey"].get<std::string>();
    if (j.contains("pollIntervalMinutes")) pollIntervalMinutes = j["pollIntervalMinutes"].get<int>();
    if (j.contains("raidPanel")) raidPanel = WindowFromJson(j["raidPanel"], raidPanel);
    if (j.contains("strikesPanel")) strikesPanel = WindowFromJson(j["strikesPanel"], strikesPanel);
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
    if (j.contains("organicGridBoxBackgrounds")) {
        organicGridBoxBackgrounds = j["organicGridBoxBackgrounds"].get<bool>();
    }
    if (j.contains("colorCleared")) colorCleared = ColorFromJson(j["colorCleared"], colorCleared);
    if (j.contains("colorNotCleared")) colorNotCleared = ColorFromJson(j["colorNotCleared"], colorNotCleared);
    if (j.contains("colorUnknown")) colorUnknown = ColorFromJson(j["colorUnknown"], colorUnknown);
    if (j.contains("colorNonWeeklyBounty")) {
        colorNonWeeklyBounty = ColorFromJson(j["colorNonWeeklyBounty"], colorNonWeeklyBounty);
    }
}

void SettingsStore::Save(const std::string& path) const {
    nlohmann::json j = {
        {"apiKey", apiKey},
        {"pollIntervalMinutes", pollIntervalMinutes},
        {"raidPanel", WindowToJson(raidPanel)},
        {"strikesPanel", WindowToJson(strikesPanel)},
        {"panelLayout", panelLayout == PanelLayout::Horizontal ? "Horizontal" : "Vertical"},
        {"keybindToggleRaids", keybindToggleRaids},
        {"keybindToggleStrikes", keybindToggleStrikes},
        {"cornerIconEnabled", cornerIconEnabled},
        {"highlightNonWeeklyBounty", highlightNonWeeklyBounty},
        {"omitEventEncounters", omitEventEncounters},
        {"anchorStrikesToRaidPanel", anchorStrikesToRaidPanel},
        {"organicGridBoxBackgrounds", organicGridBoxBackgrounds},
        {"colorCleared", ColorToJson(colorCleared)},
        {"colorNotCleared", ColorToJson(colorNotCleared)},
        {"colorUnknown", ColorToJson(colorUnknown)},
        {"colorNonWeeklyBounty", ColorToJson(colorNonWeeklyBounty)},
    };

    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    std::ofstream out(path);
    out << j.dump(2);
}

}  // namespace rc
