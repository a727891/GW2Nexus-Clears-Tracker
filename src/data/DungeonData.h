#pragma once

#include "core/Types.h"

#include <array>
#include <string>
#include <vector>

namespace rc {

struct DungeonPathDef {
    std::string id;
    std::string name;
    std::string abbreviation;
};

struct DungeonGroupDef {
    std::string name;
    std::string abbreviation;
    std::vector<DungeonPathDef> paths;
    bool isFrequenterSummary = false;
};

class DungeonData {
public:
    static constexpr const char* kFrequenterId = "freq";
    static constexpr int kFrequenterIndex = 8;

    static const std::array<DungeonGroupDef, 9>& Groups();
};

}  // namespace rc
