#pragma once

#include "core/Types.h"
#include "core/SettingsStore.h"

#include <imgui.h>

namespace rc {

class AppState;

namespace OptionsUiKit {

constexpr float kSidebarWidth = 180.0f;
constexpr float kContentBgAlpha = 0.22f;

struct ShellState {
    int generalSection = 0;
    int raidsSection = 0;
    int strikesSection = 0;
    int fractalsSection = 0;
    int dungeonsSection = 0;
};

ImVec4 GoldColor();
ImVec4 GrayColor();
ImVec4 WhiteColor();

void SectionHeading(const char* text);
void SectionSubtext(const char* text);
void WarningText(const char* text);
void DisabledGateText(const char* text);

bool SettingCheckbox(const char* label, bool* value, const char* tooltip = nullptr);
bool SettingSliderFloat(const char* label, float* value, float min, float max,
                        const char* format = "%.2f", const char* tooltip = nullptr);
bool SettingSliderInt(const char* label, int* value, int min, int max,
                      const char* tooltip = nullptr);
bool SettingColorRgb(const char* label, ColorRGB& color, const char* tooltip = nullptr);
bool SettingCombo(const char* label, int* currentIndex, const char* const* items, int itemCount,
                  const char* tooltip = nullptr);

bool BeginTabPage(int& sectionIndex, const char* const* sectionLabels, int sectionCount);
void EndTabPage();

void BeginContentPanel(ImTextureID backgroundTexture = nullptr);
void EndContentPanel();

void RenderExpansionBanner(const char* assetFilename, AppState& state);

void RenderGridPreview(const SettingsStore& settings);

}  // namespace OptionsUiKit
}  // namespace rc
