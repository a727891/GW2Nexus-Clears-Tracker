#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "ui/EncounterSelectionPanel.h"
#include "ui/LabelCustomizationPanel.h"
#include "ui/MentorProgressPopupService.h"
#include "ui/OptionsTextureService.h"
#include "ui/OptionsUiKit.h"

namespace rc {
namespace OptionsRaidsTab {
namespace {

enum class Section { Options = 0, Selection, CustomizeLabels };

void RenderOptions(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Raid Options");

    SettingCheckbox("Show raid panel", &state.settings.raidPanel.visible);

    SettingCheckbox("Show raid mentor progress in tooltips", &state.settings.showMentorProgress,
                    "Shows raid mentor achievement progress in raid boss tooltips. Requires a "
                    "registered GW2 API key with account and progression permissions.");

    SettingCheckbox("Show mentor progress popup", &state.settings.showMentorProgressPopup,
                    "Shows a popup when raid mentor achievement progress increases. Requires "
                    "mentor progress in tooltips to be enabled.");

    if (SettingCheckbox("Reposition mentor progress popup",
                        &state.settings.mentorProgressPopupReposition,
                        "Shows a draggable example popup to set where mentor progress popups "
                        "appear.")) {
        if (!state.settings.mentorProgressPopupReposition) {
            MentorProgressPopupService::SaveExamplePosition(state);
        }
    }
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"Options", "Selection", "Customize Labels"};

    BeginTabPage(section, kSections, 3);
    ImGui::PushID("raids");
    BeginContentPanel(OptionsTextureService::BackgroundTexture());

    switch (static_cast<Section>(section)) {
        case Section::Options:
            RenderOptions(state);
            break;
        case Section::Selection:
            EncounterSelectionPanel::RenderRaidSelection(state);
            break;
        case Section::CustomizeLabels:
            LabelCustomizationPanel::RenderRaidLabels(state);
            break;
    }

    EndContentPanel();
    ImGui::PopID();
    EndTabPage();
}

}  // namespace OptionsRaidsTab
}  // namespace rc
