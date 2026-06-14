#include "ui/FractalCmTooltip.h"

#include "services/DatAssetIconService.h"
#include "services/PriorityRotationService.h"

#include <algorithm>
#include <imgui.h>

namespace rc {
namespace FractalCmTooltip {
namespace {

constexpr float kTooltipWidth = 370.0f;
constexpr float kIconSize = 32.0f;
constexpr float kPadding = 5.0f;
constexpr float kHeaderHeight = 52.0f;
constexpr float kRowHeight = 32.0f;
constexpr float kColumnWidth = 165.0f;

void DrawInstabilityRow(ImDrawList* draw,
                        float x,
                        float y,
                        ImTextureID icon,
                        const char* label) {
    if (icon) {
        draw->AddImage(icon, ImVec2(x, y), ImVec2(x + kIconSize, y + kIconSize));
    }
    draw->AddText(ImVec2(x + kIconSize + kPadding, y + 6.0f), IM_COL32(255, 255, 255, 255), label);
}

int TooltipHeight(size_t todayCount, size_t tomorrowCount) {
    const size_t rows = std::max(todayCount, tomorrowCount);
    const int rowBlock = static_cast<int>(rows) * static_cast<int>(kRowHeight);
    return static_cast<int>(kHeaderHeight) + 24 + rowBlock + static_cast<int>(kPadding);
}

}  // namespace

void ShowIfHovered(const ImVec2& p0,
                   const ImVec2& p1,
                   const FractalMap& map,
                   const std::string& customAbbrev,
                   int scale,
                   const FractalMapData& fractalMapData,
                   const InstabilitiesData& instabilitiesData) {
    if (!ImGui::IsMouseHoveringRect(p0, p1)) return;

    const int day = DayOfYearIndex();
    const auto todayInstabs = instabilitiesData.GetInstabsForLevelOnDay(scale, day);
    const auto tomorrowInstabs = instabilitiesData.GetInstabsForLevelOnDay(scale, (day + 1) % 366);
    const int height = TooltipHeight(todayInstabs.size(), tomorrowInstabs.size());

    ImGui::SetNextWindowSize(ImVec2(kTooltipWidth, static_cast<float>(height)));
    ImGui::BeginTooltip();

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    const std::string title = map.label + " (" + customAbbrev + ")";
    char scaleText[32];
    snprintf(scaleText, sizeof(scaleText), "Scale %d", scale);

    draw->AddText(origin, IM_COL32(255, 212, 163, 255), title.c_str());
    draw->AddText(ImVec2(origin.x, origin.y + ImGui::GetTextLineHeight()),
                  IM_COL32(255, 255, 255, 204), scaleText);

    const float sectionY = origin.y + kHeaderHeight;
    const float leftX = origin.x + kPadding;
    const float rightX = origin.x + kColumnWidth + kPadding;

    draw->AddText(ImVec2(leftX, sectionY), IM_COL32(127, 255, 0, 255), "Instabilities");
    draw->AddText(ImVec2(rightX, sectionY), IM_COL32(127, 255, 0, 255), "Tomorrow");

    const float listY = sectionY + ImGui::GetTextLineHeight() + kPadding;
    for (size_t i = 0; i < todayInstabs.size(); ++i) {
        const ImTextureID icon =
            DatAssetIconService::Request(fractalMapData.GetInstabilityAssetId(todayInstabs[i]));
        DrawInstabilityRow(draw, leftX, listY + static_cast<float>(i) * kRowHeight, icon,
                           todayInstabs[i].c_str());
    }
    for (size_t i = 0; i < tomorrowInstabs.size(); ++i) {
        const ImTextureID icon =
            DatAssetIconService::Request(fractalMapData.GetInstabilityAssetId(tomorrowInstabs[i]));
        DrawInstabilityRow(draw, rightX, listY + static_cast<float>(i) * kRowHeight, icon,
                           tomorrowInstabs[i].c_str());
    }

    ImGui::Dummy(ImVec2(kTooltipWidth - kPadding * 2.0f,
                        static_cast<float>(height) - kHeaderHeight - kPadding));
    ImGui::EndTooltip();
}

}  // namespace FractalCmTooltip
}  // namespace rc
