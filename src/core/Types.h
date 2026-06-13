#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace rc {

enum class ClearState {
    Unknown,
    Cleared,
    NotCleared,
};

struct ColorRGB {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    static ColorRGB FromHex(const std::string& hex);
    uint32_t ToImU32(float alpha = 1.0f) const;
};

struct WindowState {
    bool visible = true;
    float posX = 100.0f;
    float posY = 100.0f;
};

struct EncounterCell {
    std::string id;
    std::string name;
    std::string abbreviation;
    ClearState state = ClearState::Unknown;
    int dailyBountyAchievementId = 0;
};

struct GridGroup {
    std::string id;
    std::string name;
    std::string abbreviation;
    std::vector<EncounterCell> encounters;
    bool isDailyBounty = false;
    bool isTomorrowBounty = false;
};

}  // namespace rc
