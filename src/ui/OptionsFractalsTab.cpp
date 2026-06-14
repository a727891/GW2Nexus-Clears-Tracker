#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "ui/LabelCustomizationPanel.h"
#include "ui/OptionsCommon.h"
#include "ui/OptionsUiKit.h"

#include <imgui.h>
#include <filesystem>
#include <mutex>

namespace rc {
namespace OptionsFractalsTab {
namespace {

enum class Section { Options = 0, CustomizeLabels };

void RenderOptions(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Fractal Options");

    SettingCheckbox("Show fractals panel", &state.settings.fractalsPanel.visible);

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
    SectionSubtext("Choose which fractal rows appear on the panel.");

    BeginExpansionRow("core");

    if (SettingCheckbox("Show challenge motes", &state.settings.fractalChallengeMotes,
                        "Display CM fractals. Hover tooltip shows today's and tomorrow's "
                        "instabilities.")) {
        if (state.fractalDataReady) {
            std::lock_guard lock(state.dataMutex);
            state.RebuildFractalGroups();
        }
    }

    if (state.settings.fractalChallengeMotes && state.fractalDataReady) {
        ImGui::Indent();
        std::lock_guard lock(state.dataMutex);
        for (const int scale : state.fractalMapData.challengeMotes) {
            const auto map = state.fractalMapData.GetFractalForScale(scale);
            if (!map.IsValid()) continue;

            bool visible = state.fractalPersist.IsChallengeMoteVisible(map.apiLabel);
            if (ImGui::Checkbox(map.label.c_str(), &visible)) {
                state.fractalPersist.SetChallengeMoteVisible(map.apiLabel, visible);
                state.RebuildFractalGroups();
                const auto path =
                    (std::filesystem::path(state.addonDir) / "clearsTracker" /
                     "fractal_clears.json")
                        .string();
                state.fractalPersist.Save(path);
            }
        }
        ImGui::Unindent();
    }

    if (SettingCheckbox("Show daily tier fractals", &state.settings.fractalDailyTierN)) {
        if (state.fractalDataReady) {
            std::lock_guard lock(state.dataMutex);
            state.RebuildFractalGroups();
        }
    }
    if (SettingCheckbox("Show daily recommended fractals", &state.settings.fractalDailyRecs)) {
        if (state.fractalDataReady) {
            std::lock_guard lock(state.dataMutex);
            state.RebuildFractalGroups();
        }
    }
    if (SettingCheckbox("Show tomorrow tier fractals", &state.settings.fractalTomorrowTierN)) {
        if (state.fractalDataReady) {
            std::lock_guard lock(state.dataMutex);
            state.RebuildFractalGroups();
        }
    }

    EndExpansionRow();
}

}  // namespace

void Render(AppState& state, int& section) {
    using namespace OptionsUiKit;

    static const char* kSections[] = {"Options", "Customize Labels"};

    BeginTabPage(section, kSections, 2);
    ImGui::PushID("fractals");
    BeginContentPanel(nullptr);

    switch (static_cast<Section>(section)) {
        case Section::Options:
            RenderOptions(state);
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
