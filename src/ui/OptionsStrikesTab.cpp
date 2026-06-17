#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "ui/EncounterSelectionPanel.h"
#include "ui/LabelCustomizationPanel.h"
#include "ui/OptionsCommon.h"
#include "ui/OptionsPanelAppearance.h"
#include "ui/OptionsUiKit.h"

namespace rc {
namespace OptionsStrikesTab {
namespace {

enum class Section { OptionsAndStyle = 0, Selection, CustomizeLabels };

void RenderOptionsAndStyle(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Raid Encounters Options");

    SettingCheckbox("Show raid encounters panel", &state.settings.strikesPanel.visible);

    SettingCheckbox("Enable mouse hover tooltips", &state.settings.strikesEnableTooltips,
                    "Shows enhanced encounter tooltips with boss icons and mechanic indicators.");

    if (SettingCheckbox("Anchor raid encounters panel to raid panel",
                        &state.settings.anchorStrikesToRaidPanel,
                        "Keeps the raid encounters panel attached to the raid panel. Moving "
                        "either panel moves both.")) {
        if (state.settings.anchorStrikesToRaidPanel) {
            RealignAnchoredPanels(state);
        }
    }

    ImGui::Spacing();
    SectionHeading("Panel Style");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(GrayColor(),
                       "Customize this panel only. Global Appearance overwrites these settings.");
    ImGui::PopTextWrapPos();

    if (OptionsPanelAppearance::RenderFields(state.settings.strikesAppearance, state.settings,
                                             true)) {
        if (state.settings.anchorStrikesToRaidPanel ||
            state.settings.anchorFractalsToStrikesPanel) {
            RealignAnchoredPanels(state);
        }
    }
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"Options and Style", "Selection", "Customize Labels"};

    BeginTabPage(section, kSections, 3);
    ImGui::PushID("strikes");
    BeginContentPanel(nullptr);

    switch (static_cast<Section>(section)) {
        case Section::OptionsAndStyle:
            RenderOptionsAndStyle(state);
            break;
        case Section::Selection:
            EncounterSelectionPanel::RenderStrikeSelection(state);
            break;
        case Section::CustomizeLabels:
            LabelCustomizationPanel::RenderStrikeLabels(state);
            break;
    }

    EndContentPanel();
    ImGui::PopID();
    EndTabPage();
}

}  // namespace OptionsStrikesTab
}  // namespace rc
