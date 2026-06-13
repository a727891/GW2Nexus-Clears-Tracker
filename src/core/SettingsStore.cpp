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
    if (j.contains("colorCleared")) colorCleared = ColorFromJson(j["colorCleared"], colorCleared);
    if (j.contains("colorNotCleared")) colorNotCleared = ColorFromJson(j["colorNotCleared"], colorNotCleared);
    if (j.contains("colorUnknown")) colorUnknown = ColorFromJson(j["colorUnknown"], colorUnknown);
}

void SettingsStore::Save(const std::string& path) const {
    nlohmann::json j = {
        {"apiKey", apiKey},
        {"pollIntervalMinutes", pollIntervalMinutes},
        {"raidPanel", WindowToJson(raidPanel)},
        {"strikesPanel", WindowToJson(strikesPanel)},
        {"colorCleared", ColorToJson(colorCleared)},
        {"colorNotCleared", ColorToJson(colorNotCleared)},
        {"colorUnknown", ColorToJson(colorUnknown)},
    };

    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    std::ofstream out(path);
    out << j.dump(2);
}

}  // namespace rc
