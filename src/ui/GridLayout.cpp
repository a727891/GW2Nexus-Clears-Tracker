#include "ui/GridLayout.h"

#include <algorithm>

namespace rc {
namespace GridLayout {

namespace {

float LabelWidth(float scale) { return Scaled(kLabelColumnWidth, scale); }
float CellWidth(float scale) { return Scaled(kCellWidth, scale); }
float CellHeight(float scale) { return Scaled(kCellHeight, scale); }
float GroupSpacing(float scale) { return Scaled(kGroupSpacing, scale); }

ImVec2 MeasureGroupSize(const GridGroup& group, PanelLayout layout, float scale,
                        bool showGroupLabel) {
    const float n = static_cast<float>(group.encounters.size());
    if (layout == PanelLayout::Vertical) {
        const float labelWidth = showGroupLabel ? LabelWidth(scale) : 0.0f;
        return {labelWidth + n * CellWidth(scale), CellHeight(scale)};
    }
    const float rows = showGroupLabel ? 1.0f + n : n;
    return {CellWidth(scale), CellHeight(scale) * rows};
}

void BuildGroupCells(GroupPlacement& placement, const GridGroup& group, PanelLayout layout,
                     float scale, bool showGroupLabel) {
    const float labelWidth = LabelWidth(scale);
    const float cellWidth = CellWidth(scale);
    const float cellHeight = CellHeight(scale);

    placement.cells.clear();
    if (layout == PanelLayout::Vertical) {
        placement.labelAlign = LabelAlign::Right;
        float encounterX = 0.0f;
        if (showGroupLabel) {
            placement.cells.push_back({ImVec2(0.0f, 0.0f), true, 0, labelWidth});
            encounterX = labelWidth;
        }
        for (size_t i = 0; i < group.encounters.size(); ++i) {
            const float x = encounterX + static_cast<float>(i) * cellWidth;
            placement.cells.push_back({ImVec2(x, 0.0f), false, i, cellWidth});
        }
        return;
    }

    placement.labelAlign = LabelAlign::Center;
    size_t row = 0;
    if (showGroupLabel) {
        placement.cells.push_back({ImVec2(0.0f, 0.0f), true, 0, cellWidth});
        row = 1;
    }
    for (size_t i = 0; i < group.encounters.size(); ++i) {
        const float y = cellHeight * static_cast<float>(row + i);
        placement.cells.push_back({ImVec2(0.0f, y), false, i, cellWidth});
    }
}

}  // namespace

PanelPlacement ComputePlacement(const std::vector<GridGroup>& groups, PanelLayout layout,
                                float scale, GroupLabelDisplay labelDisplay) {
    PanelPlacement panel;
    if (groups.empty() || scale <= 0.0f) return panel;

    const bool showGroupLabel = labelDisplay != GroupLabelDisplay::Hidden;

    float cursorX = 0.0f;
    float cursorY = 0.0f;
    float maxWidth = 0.0f;
    float maxHeight = 0.0f;
    const float spacing = GroupSpacing(scale);

    panel.groups.reserve(groups.size());
    for (const auto& group : groups) {
        if (group.encounters.empty()) continue;

        GroupPlacement placement;
        placement.size = MeasureGroupSize(group, layout, scale, showGroupLabel);
        BuildGroupCells(placement, group, layout, scale, showGroupLabel);

        if (layout == PanelLayout::Vertical) {
            placement.origin = ImVec2(0.0f, cursorY);
            cursorY += placement.size.y + spacing;
            maxWidth = std::max(maxWidth, placement.size.x);
        } else {
            placement.origin = ImVec2(cursorX, 0.0f);
            cursorX += placement.size.x + spacing;
            maxHeight = std::max(maxHeight, placement.size.y);
        }

        panel.groups.push_back(std::move(placement));
    }

    if (layout == PanelLayout::Vertical) {
        panel.contentSize = {maxWidth, std::max(0.0f, cursorY - spacing)};
    } else {
        panel.contentSize = {std::max(0.0f, cursorX - spacing), maxHeight};
    }

    return panel;
}

}  // namespace GridLayout
}  // namespace rc
