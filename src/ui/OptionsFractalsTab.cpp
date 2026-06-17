#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "ui/EncounterSelectionPanel.h"
#include "ui/LabelCustomizationPanel.h"
#include "ui/OptionsCommon.h"
#include "ui/OptionsPanelAppearance.h"
#include "ui/OptionsUiKit.h"

namespace rc {
namespace OptionsFractalsTab {
namespace {

enum class Section { OptionsAndStyle = 0, Selection, CustomizeLabels };

void RenderOptionsAndStyle(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Fractal Options");

    SettingCheckbox("Show fractals panel", &state.settings.fractalsPanel.visible);

    SettingCheckbox("Enable mouse hover tooltips", &state.settings.fractalsEnableTooltips,
                    "Shows enhanced encounter tooltips with boss icons and mechanic indicators.");

    if (SettingCheckbox("Anchor fractals panel to raid encounters panel",
                        &state.settings.anchorFractalsToStrikesPanel,
                        "Keeps the fractals panel attached to the raid encounters panel. Moving "
                        "either panel moves both, and chained raid anchoring is preserved when "
                        "enabled.")) {
        if (state.settings.anchorFractalsToStrikesPanel) {
            RealignAnchoredPanels(state);
        }
    }

    ImGui::Spacing();
    SectionHeading("Panel Style");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(GrayColor(),
                       "Customize this panel only. Global Appearance overwrites these settings.");
    ImGui::PopTextWrapPos();

    if (OptionsPanelAppearance::RenderFields(state.settings.fractalsAppearance, state.settings,
                                             true)) {
        if (state.settings.anchorFractalsToStrikesPanel) {
            RealignAnchoredPanels(state);
        }
    }
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"Options and Style", "Selection", "Customize Labels"};

    BeginTabPage(section, kSections, 3);
    ImGui::PushID("fractals");
    BeginContentPanel(nullptr);

    switch (static_cast<Section>(section)) {
        case Section::OptionsAndStyle:
            RenderOptionsAndStyle(state);
            break;
        case Section::Selection:
            EncounterSelectionPanel::RenderFractalSelection(state);
            break;
        case Section::CustomizeLabels:
            LabelCustomizationPanel::RenderFractalLabels(state);
            break;
    }

    EndContentPanel();
    ImGui::PopID();
    EndTabPage();
}

}  // namespace OptionsFractalsTab
}  // namespace rc
