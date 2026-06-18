#pragma once

#include <cstdint>

namespace rc {

// Nexus signature 0x60018001 (Clears, index 01)
inline constexpr int32_t kSignature = 0x60018001;
inline constexpr const char* kDisplayName = "Clears Tracker";
inline constexpr const char* kLogTag = "Clears Tracker";
inline constexpr const char* kAuthor = "Soeed.4160";
inline constexpr const char* kDescription =
    "Track your daily and weekly PvE instance clears. Raids, Fractals, and Dungeons.";
inline constexpr const char* kPatchNotesUrl = "https://pkgs.blishhud.com/Soeed.RaidClears.html";
inline constexpr const char* kUpdateLink = "https://github.com/a727891/GW2Nexus-Clears-Tracker";

}  // namespace rc
