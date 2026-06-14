#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "ui/OptionsTextureService.h"
#include "ui/OptionsUiKit.h"

#include <imgui.h>
#include <mutex>

namespace rc {
namespace OptionsDungeonsTab {
namespace {

enum class Section { Options = 0, Colors };

void RenderOptions(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Dungeon Options");

    SettingCheckbox("Show dungeons panel", &state.settings.dungeonsPanel.visible);

    SettingCheckbox("Show frequenter summary row", &state.settings.dungeonFrequenterVisible);
    SettingCheckbox("Highlight frequented paths", &state.settings.dungeonHighlightFrequenter);

    ImGui::Spacing();
    SectionSubtext("Toggle individual dungeon columns on the panel.");

    static const char* kDungeonNames[] = {"AC", "CM", "TA", "SE", "CoF", "HW", "CoE", "Arah"};
    for (size_t i = 0; i < state.settings.dungeonVisible.size(); ++i) {
        ImGui::Checkbox(kDungeonNames[i], &state.settings.dungeonVisible[i]);
    }
}

void RenderColors(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Dungeon Colors");
    SectionSubtext("Color used when frequented dungeon paths are highlighted.");

    if (SettingColorRgb("Frequenter text color", state.settings.colorDungeonFrequenter)) {
        std::lock_guard lock(state.dataMutex);
        state.ApplyDungeonClears();
    }
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"Options", "Colors"};

    BeginTabPage(section, kSections, 2);
    ImGui::PushID("dungeons");
    BeginContentPanel(OptionsTextureService::BackgroundTexture());

    switch (static_cast<Section>(section)) {
        case Section::Options:
            RenderOptions(state);
            break;
        case Section::Colors:
            RenderColors(state);
            break;
    }

    EndContentPanel();
    ImGui::PopID();
    EndTabPage();
}

}  // namespace OptionsDungeonsTab
}  // namespace rc
