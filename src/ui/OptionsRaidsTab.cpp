#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "ui/EncounterSelectionPanel.h"
#include "ui/LabelCustomizationPanel.h"
#include "ui/MentorProgressPopupService.h"
#include "ui/OptionsCommon.h"
#include "ui/OptionsPanelAppearance.h"
#include "ui/OptionsUiKit.h"

#include <mutex>

namespace rc {
namespace OptionsRaidsTab {
namespace {

enum class Section { OptionsAndStyle = 0, Highlights, Selection, CustomizeLabels };

void RenderOptionsAndStyle(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Raid Options");

    SettingCheckbox("Show raid panel", &state.settings.raidPanel.visible);

    SettingCheckbox("Enable mouse hover tooltips", &state.settings.raidEnableTooltips,
                    "Shows enhanced encounter tooltips with boss icons and mechanic indicators.");

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

    ImGui::Spacing();
    SectionHeading("Panel Style");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(GrayColor(),
                       "Customize this panel only. Global Appearance overwrites these settings.");
    ImGui::PopTextWrapPos();

    if (OptionsPanelAppearance::RenderFields(state.settings.raidAppearance, state.settings, true)) {
        if (state.settings.anchorStrikesToRaidPanel || state.settings.anchorFractalsToStrikesPanel) {
            RealignAnchoredPanels(state);
        }
    }
}

void RenderHighlights(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Highlights");
    SectionSubtext("Weekly raid wing and bounty highlight behavior.");

    if (SettingCheckbox("Highlight non-weekly bounty encounters",
                        &state.settings.highlightNonWeeklyBounty,
                        "Uncleared raid and raid encounter bosses that will not be daily bounties "
                        "for the rest of the week are highlighted on the raid and raid encounters "
                        "panels.")) {
        std::lock_guard lock(state.dataMutex);
        state.ApplyNonWeeklyHighlights();
    }

    if (SettingCheckbox("Omit event encounters from highlight", &state.settings.omitEventEncounters,
                        "Excludes raid event encounters from non-weekly bounty highlighting.")) {
        std::lock_guard lock(state.dataMutex);
        state.ApplyNonWeeklyHighlights();
    }

    SettingCheckbox("Highlight Emboldened raid wing", &state.settings.highlightEmbolden,
                    "Colors wing and encounter labels for the weekly Emboldened raid wing.");

    SettingCheckbox("Highlight Call of the Mists raid wing", &state.settings.highlightCotm,
                    "Colors wing and encounter labels for the weekly Call of the Mists raid wing.");

    ImGui::Spacing();
    SectionHeading("Highlight Colors");
    SettingColorRgb("Non-weekly bounty color", state.settings.colorNonWeeklyBounty,
                    "Color for encounters highlighted as non-weekly bounties.");
    SettingColorRgb("Emboldened text color", state.settings.colorEmbolden);
    SettingColorRgb("Call of the Mists text color", state.settings.colorCotm);
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"Options and Style", "Highlights", "Selection",
                                      "Customize Labels"};

    BeginTabPage(section, kSections, 4);
    ImGui::PushID("raids");
    BeginContentPanel(nullptr);

    switch (static_cast<Section>(section)) {
        case Section::OptionsAndStyle:
            RenderOptionsAndStyle(state);
            break;
        case Section::Highlights:
            RenderHighlights(state);
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
