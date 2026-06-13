#include "ui/GridLayout.h"

#include <algorithm>

namespace rc {
namespace GridLayout {

namespace {

ImVec2 MeasureGroupSize(const GridGroup& group, PanelLayout layout) {
    const float n = static_cast<float>(group.encounters.size());
    if (layout == PanelLayout::Vertical) {
        return {kLabelColumnWidth + n * kCellWidth, kCellHeight};
    }
    return {kCellWidth, kCellHeight * (1.0f + n)};
}

void BuildGroupCells(GroupPlacement& placement, const GridGroup& group, PanelLayout layout) {
    placement.cells.clear();
    if (layout == PanelLayout::Vertical) {
        placement.labelAlign = LabelAlign::Right;
        placement.cells.push_back({ImVec2(0.0f, 0.0f), true, 0, kLabelColumnWidth});
        for (size_t i = 0; i < group.encounters.size(); ++i) {
            const float x = kLabelColumnWidth + static_cast<float>(i) * kCellWidth;
            placement.cells.push_back({ImVec2(x, 0.0f), false, i, kCellWidth});
        }
        return;
    }

    placement.labelAlign = LabelAlign::Center;
    placement.cells.push_back({ImVec2(0.0f, 0.0f), true, 0, kCellWidth});
    for (size_t i = 0; i < group.encounters.size(); ++i) {
        const float y = kCellHeight * static_cast<float>(i + 1);
        placement.cells.push_back({ImVec2(0.0f, y), false, i, kCellWidth});
    }
}

}  // namespace

PanelPlacement ComputePlacement(const std::vector<GridGroup>& groups, PanelLayout layout) {
    PanelPlacement panel;
    if (groups.empty()) return panel;

    float cursorX = 0.0f;
    float cursorY = 0.0f;
    float maxWidth = 0.0f;
    float maxHeight = 0.0f;

    panel.groups.reserve(groups.size());
    for (const auto& group : groups) {
        if (group.encounters.empty()) continue;

        GroupPlacement placement;
        placement.size = MeasureGroupSize(group, layout);
        BuildGroupCells(placement, group, layout);

        if (layout == PanelLayout::Vertical) {
            placement.origin = ImVec2(0.0f, cursorY);
            cursorY += placement.size.y + kGroupSpacing;
            maxWidth = std::max(maxWidth, placement.size.x);
        } else {
            placement.origin = ImVec2(cursorX, 0.0f);
            cursorX += placement.size.x + kGroupSpacing;
            maxHeight = std::max(maxHeight, placement.size.y);
        }

        panel.groups.push_back(std::move(placement));
    }

    if (layout == PanelLayout::Vertical) {
        panel.contentSize = {maxWidth, std::max(0.0f, cursorY - kGroupSpacing)};
    } else {
        panel.contentSize = {std::max(0.0f, cursorX - kGroupSpacing), maxHeight};
    }

    return panel;
}

}  // namespace GridLayout
}  // namespace rc
