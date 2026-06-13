#include "ui/GridRenderer.h"

#include "ui/GridLayout.h"

#include <imgui.h>

namespace rc {
namespace GridRenderer {

namespace {

ImVec2 MeasureText(const char* text, ImFont* font) {
    if (font) {
        return font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, text);
    }
    return ImGui::CalcTextSize(text);
}

void DrawTextAt(ImDrawList* draw,
                const ImVec2& pos,
                ImU32 color,
                const char* text,
                ImFont* font) {
    if (font) {
        draw->AddText(font, font->FontSize, pos, color, text);
        return;
    }
    draw->AddText(pos, color, text);
}

uint32_t ColorForState(const EncounterCell& cell,
                       const SettingsStore& settings,
                       bool colorClears,
                       bool useNonWeeklyHighlight) {
    if (!colorClears || cell.state == ClearState::Unknown) {
        return settings.colorUnknown.ToImU32(0.85f);
    }
    if (cell.state == ClearState::Cleared) {
        return settings.colorCleared.ToImU32(0.85f);
    }
    if (useNonWeeklyHighlight && cell.highlightNonWeeklyBounty) {
        return settings.colorNonWeeklyBounty.ToImU32(0.85f);
    }
    return settings.colorNotCleared.ToImU32(0.85f);
}

void DrawEncounterCellAt(const ImVec2& p0,
                         float width,
                         const EncounterCell& cell,
                         const SettingsStore& settings,
                         bool colorClears,
                         bool useNonWeeklyHighlight,
                         ImFont* font) {
    const ImVec2 p1(p0.x + width, p0.y + GridLayout::kCellHeight);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(p0, p1,
                        ColorForState(cell, settings, colorClears, useNonWeeklyHighlight));
    draw->AddRect(p0, p1, IM_COL32(0, 0, 0, 180));

    const char* label = cell.abbreviation.empty() ? cell.name.c_str() : cell.abbreviation.c_str();
    const ImVec2 textSize = MeasureText(label, font);
    const ImVec2 textPos(p0.x + (width - textSize.x) * 0.5f,
                         p0.y + (GridLayout::kCellHeight - textSize.y) * 0.5f);
    DrawTextAt(draw, textPos, IM_COL32(255, 255, 255, 255), label, font);

    if (ImGui::IsMouseHoveringRect(p0, p1)) {
        ImGui::SetTooltip("%s", cell.name.c_str());
    }
}

void DrawLabelCellAt(const ImVec2& p0,
                     float width,
                     const char* abbreviation,
                     const char* tooltip,
                     GridLayout::LabelAlign align,
                     ImFont* font) {
    const ImVec2 p1(p0.x + width, p0.y + GridLayout::kCellHeight);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(p0, p1, IM_COL32(40, 40, 40, 220));
    draw->AddRect(p0, p1, IM_COL32(0, 0, 0, 180));

    const ImVec2 textSize = MeasureText(abbreviation, font);
    float textX = p0.x + (width - textSize.x) * 0.5f;
    if (align == GridLayout::LabelAlign::Right) {
        textX = p0.x + width - textSize.x - 4.0f;
    }
    DrawTextAt(draw, ImVec2(textX, p0.y + (GridLayout::kCellHeight - textSize.y) * 0.5f),
               IM_COL32(255, 255, 255, 255), abbreviation, font);
    if (ImGui::IsMouseHoveringRect(p0, p1)) {
        ImGui::SetTooltip("%s", tooltip);
    }
}

void DrawGroupAt(const ImVec2& contentOrigin,
                 const GridGroup& group,
                 const GridLayout::GroupPlacement& placement,
                 const SettingsStore& settings,
                 bool colorClears,
                 bool useNonWeeklyHighlight,
                 ImFont* font) {
    const char* wingLabel =
        group.abbreviation.empty() ? group.name.c_str() : group.abbreviation.c_str();
    const bool applyColors = colorClears && !group.isTomorrowBounty;

    for (const auto& cell : placement.cells) {
        const ImVec2 p0(contentOrigin.x + placement.origin.x + cell.position.x,
                        contentOrigin.y + placement.origin.y + cell.position.y);
        if (cell.isLabel) {
            DrawLabelCellAt(p0, cell.width, wingLabel, group.name.c_str(), placement.labelAlign,
                            font);
        } else if (cell.encounterIndex < group.encounters.size()) {
            DrawEncounterCellAt(p0, cell.width, group.encounters[cell.encounterIndex], settings,
                                applyColors, useNonWeeklyHighlight, font);
        }
    }
}

}  // namespace

ImVec2 DrawGroups(const std::vector<GridGroup>& groups,
                  const SettingsStore& settings,
                  bool colorClears,
                  bool useNonWeeklyHighlight,
                  ImFont* font) {
    const PanelLayout layout = settings.panelLayout;
    const auto placement = GridLayout::ComputePlacement(groups, layout);
    const ImVec2 contentOrigin = ImGui::GetCursorScreenPos();

    ImGui::Dummy(placement.contentSize);

    size_t placementIndex = 0;
    for (const auto& group : groups) {
        if (group.encounters.empty()) continue;
        if (placementIndex >= placement.groups.size()) break;
        DrawGroupAt(contentOrigin, group, placement.groups[placementIndex], settings, colorClears,
                    useNonWeeklyHighlight, font);
        ++placementIndex;
    }

    return placement.contentSize;
}

}  // namespace GridRenderer
}  // namespace rc
