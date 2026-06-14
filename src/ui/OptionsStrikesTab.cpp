#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "ui/EncounterSelectionPanel.h"
#include "ui/LabelCustomizationPanel.h"
#include "ui/OptionsCommon.h"
#include "ui/OptionsUiKit.h"

namespace rc {
namespace OptionsStrikesTab {
namespace {

enum class Section { Options = 0, Selection, CustomizeLabels };

void RenderOptions(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Raid Encounters Options");

    SettingCheckbox("Show raid encounters panel", &state.settings.strikesPanel.visible);

    if (SettingCheckbox("Anchor raid encounters panel to raid panel",
                        &state.settings.anchorStrikesToRaidPanel,
                        "Keeps the raid encounters panel attached to the raid panel. Moving "
                        "either panel moves both.")) {
        if (state.settings.anchorStrikesToRaidPanel) {
            RealignAnchoredPanels(state);
        }
    }
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"Options", "Selection", "Customize Labels"};

    BeginTabPage(section, kSections, 3);
    ImGui::PushID("strikes");
    BeginContentPanel(nullptr);

    switch (static_cast<Section>(section)) {
        case Section::Options:
            RenderOptions(state);
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
