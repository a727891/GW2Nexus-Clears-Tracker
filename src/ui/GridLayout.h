#pragma once

#include "core/Types.h"

#include <imgui.h>
#include <cstddef>
#include <vector>

namespace rc {
namespace GridLayout {

constexpr float kLabelColumnWidth = 48.0f;
constexpr float kCellWidth = 40.0f;
constexpr float kCellHeight = 28.0f;
constexpr float kGroupSpacing = 4.0f;

enum class LabelAlign {
    Center,
    Right,
};

struct CellPlacement {
    ImVec2 position{};
    bool isLabel = false;
    size_t encounterIndex = 0;
    float width = kCellWidth;
};

struct GroupPlacement {
    ImVec2 origin{};
    ImVec2 size{};
    LabelAlign labelAlign = LabelAlign::Right;
    std::vector<CellPlacement> cells;
};

struct PanelPlacement {
    ImVec2 contentSize{};
    std::vector<GroupPlacement> groups;
};

PanelPlacement ComputePlacement(const std::vector<GridGroup>& groups, PanelLayout layout,
                                float scale = 1.0f);

inline float Scaled(float value, float scale) { return value * scale; }

}  // namespace GridLayout
}  // namespace rc
