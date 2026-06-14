#include "ui/GridRenderer.h"

#include "services/WeeklyModifierService.h"
#include "ui/EncounterTooltip.h"
#include "ui/GridLayout.h"
#include "ui/GridMaskService.h"

#include <imgui.h>
#include <optional>
#include <string>

namespace rc {
namespace GridRenderer {

namespace {

float EffectiveFontSize(ImFont* font, float scale) {
    if (font) return font->FontSize * scale;
    return ImGui::GetFontSize() * scale;
}

ImVec2 MeasureText(const char* text, ImFont* font, float fontSize) {
    if (font) {
        return font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    }
    return ImGui::CalcTextSize(text);
}

void DrawTextAt(ImDrawList* draw,
                const ImVec2& pos,
                ImU32 color,
                const char* text,
                ImFont* font,
                float fontSize) {
    if (font) {
        draw->AddText(font, fontSize, pos, color, text);
        return;
    }
    draw->AddText(pos, color, text);
}

void DrawBoldTextAt(ImDrawList* draw,
                    const ImVec2& pos,
                    ImU32 color,
                    const char* text,
                    ImFont* font,
                    float fontSize) {
    const float boldOffset = fontSize >= 14.0f ? 1.0f : 0.5f;
    if (font) {
        draw->AddText(font, fontSize, ImVec2(pos.x + boldOffset, pos.y), color, text);
        draw->AddText(font, fontSize, pos, color, text);
        return;
    }
    draw->AddText(ImVec2(pos.x + boldOffset, pos.y), color, text);
    draw->AddText(pos, color, text);
}

uint32_t ApplyOpacity(uint32_t color, float opacity) {
    if (opacity >= 1.0f) return color;
    if (opacity <= 0.0f) return color & ~IM_COL32_A_MASK;
    const uint32_t alpha = (color >> IM_COL32_A_SHIFT) & 0xFF;
    const uint32_t scaledAlpha = static_cast<uint32_t>(alpha * opacity);
    return (color & ~IM_COL32_A_MASK) | (scaledAlpha << IM_COL32_A_SHIFT);
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

uint32_t LabelTextColor(const GridGroup& group,
                        const SettingsStore& settings,
                        const RaidData* raidData) {
    if (!raidData || group.isDailyBounty || group.isTomorrowBounty) {
        return settings.colorText.ToImU32(1.0f);
    }

    const RaidWing* wing = raidData->GetWingById(group.id);
    if (!wing) return settings.colorText.ToImU32(1.0f);

    const auto modifier = WeeklyModifierService::ForWing(*wing, raidData->secondsInWeek);
    if (modifier.emboldened && settings.highlightEmbolden) {
        return settings.colorEmbolden.ToImU32(1.0f);
    }
    if (modifier.callOfTheMists && settings.highlightCotm) {
        return settings.colorCotm.ToImU32(1.0f);
    }
    return settings.colorText.ToImU32(1.0f);
}

std::string GroupLabelText(const GridGroup& group, GroupLabelDisplay /*mode*/) {
    if (!group.abbreviation.empty()) return group.abbreviation;
    return group.name;
}

void DrawCellBackground(ImDrawList* draw,
                        const ImVec2& p0,
                        float width,
                        float height,
                        uint32_t fillColor,
                        bool organic,
                        uint32_t styleSeed,
                        float opacity) {
    fillColor = ApplyOpacity(fillColor, opacity);
    if (opacity <= 0.0f) return;

    const ImVec2 baseP1(p0.x + width, p0.y + height);
    if (organic && GridMaskService::HasMasks()) {
        const auto style = GridMaskService::StyleForSeed(styleSeed);
        const ImVec2 drawP0(p0.x + style.xOffset, p0.y + style.yOffset);
        const ImVec2 drawP1(baseP1.x + style.widthDelta, baseP1.y + style.heightDelta);
        draw->AddImage(style.texture, drawP0, drawP1, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                       fillColor);
        return;
    }

    draw->AddRectFilled(p0, baseP1, fillColor);
    draw->AddRect(p0, baseP1, ApplyOpacity(IM_COL32(0, 0, 0, 180), opacity));
}

void DrawEncounterCellAt(const ImVec2& p0,
                         float width,
                         float cellHeight,
                         const EncounterCell& cell,
                         const GridGroup& group,
                         const SettingsStore& settings,
                         bool colorClears,
                         bool useNonWeeklyHighlight,
                         ImFont* font,
                         float fontSize,
                         uint32_t textColor,
                         const GridDrawContext& context,
                         bool useDungeonFrequenter) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const bool applyColors =
        colorClears && !group.isTomorrowBounty && !group.isTomorrowFractal;
    const uint32_t fillColor =
        ColorForState(cell, settings, applyColors, useNonWeeklyHighlight);
    const uint32_t styleSeed =
        GridMaskService::HashSeed(group.id, cell.id.empty() ? cell.name : cell.id);
    DrawCellBackground(draw, p0, width, cellHeight, fillColor,
                       settings.organicGridBoxBackgrounds, styleSeed, settings.gridOpacity);

    const ImVec2 p1(p0.x + width, p0.y + cellHeight);
    const char* label = cell.abbreviation.empty() ? cell.name.c_str() : cell.abbreviation.c_str();
    const ImVec2 textSize = MeasureText(label, font, fontSize);
    const ImVec2 textPos(p0.x + (width - textSize.x) * 0.5f,
                         p0.y + (cellHeight - textSize.y) * 0.5f);
    uint32_t cellTextColor = textColor;
    if (useDungeonFrequenter && cell.highlightFrequenter &&
        settings.dungeonHighlightFrequenter) {
        cellTextColor = settings.colorDungeonFrequenter.ToImU32(1.0f);
    }
    DrawBoldTextAt(draw, textPos, ApplyOpacity(cellTextColor, settings.gridOpacity), label, font,
                   fontSize);

    if (settings.enableTooltips && context.raidData) {
        std::optional<EncounterTooltipData> tooltipData;
        if (context.isStrikePanel && context.strikeData) {
            tooltipData = EncounterTooltip::BuildFromStrike(*context.strikeData, cell.id);
        }
        if (!tooltipData && !context.isStrikePanel) {
            tooltipData = EncounterTooltip::BuildFromRaid(*context.raidData, cell.id);
        }
        if (!tooltipData && context.isStrikePanel) {
            tooltipData = EncounterTooltip::BuildFromRaid(*context.raidData, cell.id);
        }
        if (tooltipData) {
            if (!cell.abbreviation.empty()) {
                tooltipData->abbreviation = cell.abbreviation;
            }
            EncounterTooltip::ShowIfHovered(p0, p1, *tooltipData, *context.raidData,
                                            context.mentorProgress, settings.showMentorProgress);
        } else if (ImGui::IsMouseHoveringRect(p0, p1)) {
            ImGui::SetTooltip("%s", cell.name.c_str());
        }
    }
}

void DrawLabelCellAt(const ImVec2& p0,
                     float width,
                     float cellHeight,
                     const char* abbreviation,
                     const char* tooltip,
                     const std::string& groupId,
                     GridLayout::LabelAlign align,
                     const SettingsStore& settings,
                     ImFont* font,
                     float fontSize,
                     uint32_t textColor) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const uint32_t styleSeed = GridMaskService::HashSeed(groupId);
    DrawCellBackground(draw, p0, width, cellHeight, IM_COL32(40, 40, 40, 220),
                       settings.organicGridBoxBackgrounds, styleSeed, settings.labelOpacity);

    const ImVec2 p1(p0.x + width, p0.y + cellHeight);
    const ImVec2 textSize = MeasureText(abbreviation, font, fontSize);
    float textX = p0.x + (width - textSize.x) * 0.5f;
    if (align == GridLayout::LabelAlign::Right) {
        textX = p0.x + width - textSize.x - GridLayout::Scaled(4.0f, settings.panelScale);
    }
    DrawTextAt(draw, ImVec2(textX, p0.y + (cellHeight - textSize.y) * 0.5f),
               ApplyOpacity(textColor, settings.labelOpacity), abbreviation, font, fontSize);
    if (settings.enableTooltips && ImGui::IsMouseHoveringRect(p0, p1)) {
        ImGui::SetTooltip("%s", tooltip);
    }
}

void DrawGroupAt(const ImVec2& contentOrigin,
                 const GridGroup& group,
                 const GridLayout::GroupPlacement& placement,
                 const SettingsStore& settings,
                 bool colorClears,
                 bool useNonWeeklyHighlight,
                 ImFont* font,
                 float fontSize,
                 float cellHeight,
                 uint32_t textColor,
                 const GridDrawContext& context,
                 bool useDungeonFrequenter) {
    const std::string groupLabel = GroupLabelText(group, settings.groupLabelDisplay);

    for (const auto& cell : placement.cells) {
        const ImVec2 p0(contentOrigin.x + placement.origin.x + cell.position.x,
                        contentOrigin.y + placement.origin.y + cell.position.y);
        if (cell.isLabel) {
            DrawLabelCellAt(p0, cell.width, cellHeight, groupLabel.c_str(), group.name.c_str(),
                            group.id, placement.labelAlign, settings, font, fontSize, textColor);
        } else if (cell.encounterIndex < group.encounters.size()) {
            DrawEncounterCellAt(p0, cell.width, cellHeight,
                                group.encounters[cell.encounterIndex], group, settings, colorClears,
                                useNonWeeklyHighlight, font, fontSize, textColor, context,
                                useDungeonFrequenter);
        }
    }
}

}  // namespace

ImVec2 DrawGroups(const std::vector<GridGroup>& groups,
                  const SettingsStore& settings,
                  bool colorClears,
                  bool useNonWeeklyHighlight,
                  ImFont* font,
                  const GridDrawContext& context,
                  bool useDungeonFrequenter) {
    const float scale = settings.panelScale > 0.0f ? settings.panelScale : 1.0f;
    const float fontSize = EffectiveFontSize(font, scale);
    const float cellHeight = GridLayout::Scaled(GridLayout::kCellHeight, scale);
    const PanelLayout layout = settings.panelLayout;
    const auto placement =
        GridLayout::ComputePlacement(groups, layout, scale, settings.groupLabelDisplay);
    const ImVec2 contentOrigin = ImGui::GetCursorScreenPos();

    ImGui::Dummy(placement.contentSize);

    if (settings.panelBackgroundOpacity > 0.0f) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImVec2 panelP1(contentOrigin.x + placement.contentSize.x,
                             contentOrigin.y + placement.contentSize.y);
        draw->AddRectFilled(contentOrigin, panelP1,
                            IM_COL32(0, 0, 0,
                                     static_cast<int>(settings.panelBackgroundOpacity * 255.0f)));
    }

    size_t placementIndex = 0;
    for (const auto& group : groups) {
        if (group.encounters.empty()) continue;
        if (placementIndex >= placement.groups.size()) break;
        const uint32_t textColor = LabelTextColor(group, settings, context.raidData);
        DrawGroupAt(contentOrigin, group, placement.groups[placementIndex], settings, colorClears,
                    useNonWeeklyHighlight, font, fontSize, cellHeight, textColor, context,
                    useDungeonFrequenter);
        ++placementIndex;
    }

    return placement.contentSize;
}

}  // namespace GridRenderer
}  // namespace rc
