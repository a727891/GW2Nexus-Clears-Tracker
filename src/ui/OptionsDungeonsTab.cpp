#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "ui/EncounterSelectionPanel.h"
#include "ui/OptionsPanelAppearance.h"
#include "ui/OptionsUiKit.h"

#include <imgui.h>
#include <mutex>

namespace rc {
namespace OptionsDungeonsTab {
namespace {

enum class Section { OptionsAndStyle = 0, Selection };

void RenderOptionsAndStyle(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Dungeon Options");

    SettingCheckbox("Show dungeons panel", &state.settings.dungeonsPanel.visible);

    SettingCheckbox("Enable mouse hover tooltips", &state.settings.dungeonsEnableTooltips,
                    "Shows enhanced encounter tooltips with boss icons and mechanic indicators.");

    SettingCheckbox("Show frequenter summary row", &state.settings.dungeonFrequenterVisible);
    SettingCheckbox("Highlight frequented paths", &state.settings.dungeonHighlightFrequenter);

    ImGui::Spacing();
    SectionHeading("Panel Style");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(GrayColor(),
                       "Customize this panel only. Global Appearance overwrites these settings.");
    ImGui::PopTextWrapPos();

    OptionsPanelAppearance::RenderFields(state.settings.dungeonsAppearance, state.settings, true);

    ImGui::Spacing();
    SectionHeading("Dungeon Colors");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(GrayColor(), "Color used when frequented dungeon paths are highlighted.");
    ImGui::PopTextWrapPos();

    if (SettingColorRgb("Frequenter text color", state.settings.colorDungeonFrequenter)) {
        std::lock_guard lock(state.dataMutex);
        state.ApplyDungeonClears();
    }
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"Options and Style", "Selection"};

    BeginTabPage(section, kSections, 2);
    ImGui::PushID("dungeons");
    BeginContentPanel(nullptr);

    switch (static_cast<Section>(section)) {
        case Section::OptionsAndStyle:
            RenderOptionsAndStyle(state);
            break;
        case Section::Selection:
            EncounterSelectionPanel::RenderDungeonSelection(state);
            break;
    }

    EndContentPanel();
    ImGui::PopID();
    EndTabPage();
}

}  // namespace OptionsDungeonsTab
}  // namespace rc
