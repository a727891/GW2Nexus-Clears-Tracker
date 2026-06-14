#include "ui/OptionsUiKit.h"

#include "ui/ContentLogoService.h"
#include "ui/GridMaskService.h"

#include <imgui.h>

namespace rc {
namespace OptionsUiKit {

ImVec4 GoldColor() { return ImVec4(1.0f, 0.78f, 0.0f, 1.0f); }

ImVec4 GrayColor() { return ImVec4(0.7f, 0.7f, 0.7f, 1.0f); }

ImVec4 WhiteColor() { return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); }

void SectionHeading(const char* text) {
    ImGui::Spacing();
    ImGui::TextColored(GoldColor(), "%s", text);
    ImGui::Separator();
}

void SectionSubtext(const char* text) { ImGui::TextColored(GrayColor(), "%s", text); }

void WarningText(const char* text) {
    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "%s", text);
}

void DisabledGateText(const char* text) { ImGui::TextDisabled("%s", text); }

bool SettingCheckbox(const char* label, bool* value, const char* tooltip) {
    const bool changed = ImGui::Checkbox(label, value);
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return changed;
}

bool SettingSliderFloat(const char* label, float* value, float min, float max, const char* format,
                        const char* tooltip) {
    const bool changed = ImGui::SliderFloat(label, value, min, max, format);
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return changed;
}

bool SettingSliderInt(const char* label, int* value, int min, int max, const char* tooltip) {
    const bool changed = ImGui::SliderInt(label, value, min, max);
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return changed;
}

bool SettingColorRgb(const char* label, ColorRGB& color, const char* tooltip) {
    float rgb[3] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f};
    const bool changed = ImGui::ColorEdit3(label, rgb);
    if (changed) {
        color = {static_cast<uint8_t>(rgb[0] * 255), static_cast<uint8_t>(rgb[1] * 255),
                 static_cast<uint8_t>(rgb[2] * 255)};
    }
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return changed;
}

bool SettingCombo(const char* label, int* currentIndex, const char* const* items, int itemCount,
                  const char* tooltip) {
    const bool changed = ImGui::Combo(label, currentIndex, items, itemCount);
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return changed;
}

bool BeginTabPage(int& sectionIndex, const char* const* sectionLabels, int sectionCount) {
    ImGui::Columns(2, "##options_sidebar", false);
    ImGui::SetColumnWidth(0, kSidebarWidth);

    for (int i = 0; i < sectionCount; ++i) {
        if (ImGui::Selectable(sectionLabels[i], sectionIndex == i)) {
            sectionIndex = i;
        }
    }

    ImGui::NextColumn();
    return true;
}

void EndTabPage() { ImGui::Columns(1); }

void BeginContentPanel(ImTextureID backgroundTexture) {
    ImGui::BeginChild("##options_content", ImVec2(0, 520.0f), false,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (backgroundTexture) {
        const ImVec2 p0 = ImGui::GetWindowPos();
        const ImVec2 p1(p0.x + ImGui::GetWindowWidth(), p0.y + ImGui::GetWindowHeight());
        const ImU32 tint =
            IM_COL32(255, 255, 255, static_cast<int>(255.0f * kContentBgAlpha));
        ImGui::GetWindowDrawList()->AddImage(backgroundTexture, p0, p1, ImVec2(0.0f, 0.0f),
                                             ImVec2(1.0f, 1.0f), tint);
    }
}

void EndContentPanel() { ImGui::EndChild(); }

void BeginExpansionRow(const char* expansionId) {
    ImGui::Spacing();
    ImGui::BeginGroup();
    if (ContentLogoService::RenderLogo(expansionId)) {
        ImGui::SameLine(0.0f, 12.0f);
    }
    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
}

void EndExpansionRow() {
    ImGui::EndGroup();
    ImGui::EndGroup();
    ImGui::Spacing();
}

void RenderGridPreview(const SettingsStore& settings) {
    SectionSubtext("Preview of encounter cell styling:");
    ImGui::Spacing();

    const float cellWidth = 72.0f * settings.panelScale;
    const float cellHeight = 28.0f * settings.panelScale;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    const struct PreviewCell {
        const char* label;
        ClearState state;
    } cells[] = {{"CM", ClearState::Cleared}, {"T4", ClearState::NotCleared},
                 {"Rec", ClearState::Unknown}};

    for (size_t i = 0; i < 3; ++i) {
        const ImVec2 p0(origin.x + static_cast<float>(i) * (cellWidth + 4.0f), origin.y);
        const ImVec2 p1(p0.x + cellWidth, p0.y + cellHeight);

        uint32_t fill = settings.colorUnknown.ToImU32(settings.gridOpacity);
        if (cells[i].state == ClearState::Cleared) {
            fill = settings.colorCleared.ToImU32(settings.gridOpacity);
        } else if (cells[i].state == ClearState::NotCleared) {
            fill = settings.colorNotCleared.ToImU32(settings.gridOpacity);
        }

        if (settings.organicGridBoxBackgrounds && GridMaskService::HasMasks()) {
            const auto style = GridMaskService::StyleForSeed(static_cast<uint32_t>(i));
            const ImVec2 drawP0(p0.x + style.xOffset, p0.y + style.yOffset);
            const ImVec2 drawP1(p1.x + style.widthDelta, p1.y + style.heightDelta);
            draw->AddImage(style.texture, drawP0, drawP1, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                           fill);
        } else {
            draw->AddRectFilled(p0, p1, fill, 3.0f);
        }

        const ImVec2 textSize = ImGui::CalcTextSize(cells[i].label);
        const ImVec2 textPos(p0.x + (cellWidth - textSize.x) * 0.5f,
                              p0.y + (cellHeight - textSize.y) * 0.5f);
        draw->AddText(textPos, settings.colorText.ToImU32(1.0f), cells[i].label);
    }

    ImGui::Dummy(ImVec2(cellWidth * 3.0f + 8.0f, cellHeight + 8.0f));
}

}  // namespace OptionsUiKit
}  // namespace rc
