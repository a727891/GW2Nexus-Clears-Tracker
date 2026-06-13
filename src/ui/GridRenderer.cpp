#include "ui/GridRenderer.h"

#include <imgui.h>

namespace rc {
namespace GridRenderer {

namespace {

constexpr float kLabelColumnWidth = 48.0f;
constexpr float kCellWidth = 40.0f;
constexpr float kCellHeight = 28.0f;

uint32_t ColorForState(const EncounterCell& cell,
                       const SettingsStore& settings,
                       bool colorClears) {
    if (!colorClears || cell.state == ClearState::Unknown) {
        return settings.colorUnknown.ToImU32(0.85f);
    }
    if (cell.state == ClearState::Cleared) {
        return settings.colorCleared.ToImU32(0.85f);
    }
    return settings.colorNotCleared.ToImU32(0.85f);
}

void DrawCellAt(const ImVec2& p0,
                float width,
                const EncounterCell& cell,
                const SettingsStore& settings,
                bool colorClears) {
    const ImVec2 p1(p0.x + width, p0.y + kCellHeight);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(p0, p1, ColorForState(cell, settings, colorClears));
    draw->AddRect(p0, p1, IM_COL32(0, 0, 0, 180));

    const char* label = cell.abbreviation.empty() ? cell.name.c_str() : cell.abbreviation.c_str();
    const ImVec2 textSize = ImGui::CalcTextSize(label);
    const ImVec2 textPos(p0.x + (width - textSize.x) * 0.5f,
                         p0.y + (kCellHeight - textSize.y) * 0.5f);
    draw->AddText(textPos, IM_COL32(255, 255, 255, 255), label);

    if (ImGui::IsMouseHoveringRect(p0, p1)) {
        ImGui::SetTooltip("%s", cell.name.c_str());
    }
}

void DrawLabelCell(const char* abbreviation, const char* tooltip) {
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 p1(p0.x + kLabelColumnWidth, p0.y + kCellHeight);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(p0, p1, IM_COL32(40, 40, 40, 220));
    draw->AddRect(p0, p1, IM_COL32(0, 0, 0, 180));

    const ImVec2 textSize = ImGui::CalcTextSize(abbreviation);
    draw->AddText(ImVec2(p0.x + (kLabelColumnWidth - textSize.x) * 0.5f,
                         p0.y + (kCellHeight - textSize.y) * 0.5f),
                  IM_COL32(255, 255, 255, 255), abbreviation);
    if (ImGui::IsMouseHoveringRect(p0, p1)) {
        ImGui::SetTooltip("%s", tooltip);
    }
    ImGui::Dummy(ImVec2(kLabelColumnWidth, kCellHeight));
}

}  // namespace

void DrawGroup(const GridGroup& group, const SettingsStore& settings, bool colorClears) {
    if (group.encounters.empty()) return;

    const int columns = static_cast<int>(group.encounters.size()) + 1;
    if (!ImGui::BeginTable(group.id.c_str(), columns,
                           ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadInnerX |
                               ImGuiTableFlags_NoKeepColumnsVisible)) {
        return;
    }

    ImGui::TableSetupColumn("wing", ImGuiTableColumnFlags_WidthFixed, kLabelColumnWidth);
    for (size_t i = 0; i < group.encounters.size(); ++i) {
        ImGui::TableSetupColumn(("enc" + std::to_string(i)).c_str(),
                                ImGuiTableColumnFlags_WidthFixed, kCellWidth);
    }

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    const char* label =
        group.abbreviation.empty() ? group.name.c_str() : group.abbreviation.c_str();
    DrawLabelCell(label, group.name.c_str());

    for (size_t i = 0; i < group.encounters.size(); ++i) {
        ImGui::TableSetColumnIndex(static_cast<int>(i) + 1);
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const bool applyColors = colorClears && !group.isTomorrowBounty;
        DrawCellAt(p0, kCellWidth, group.encounters[i], settings, applyColors);
        ImGui::Dummy(ImVec2(kCellWidth, kCellHeight));
    }

    ImGui::EndTable();
    ImGui::Spacing();
}

void DrawGroups(const std::vector<GridGroup>& groups,
                const SettingsStore& settings,
                bool colorClears) {
    for (const auto& group : groups) {
        DrawGroup(group, settings, colorClears);
    }
}

}  // namespace GridRenderer
}  // namespace rc
