#include "ui/EncounterSelectionPanel.h"

#include "core/AppState.h"
#include "ui/OptionsUiKit.h"

#include <imgui.h>
#include <mutex>
#include <string>

namespace rc {
namespace EncounterSelectionPanel {
namespace {

void SaveRaidVisibility(AppState& state) {
    const auto path = state.addonDir + "/raid_settings.json";
    state.raidVisibility.Save(path);
}

void SaveStrikeVisibility(AppState& state) {
    const auto path = state.addonDir + "/strike_settings.json";
    state.strikeVisibility.Save(path);
}

}  // namespace

void RenderRaidSelection(AppState& state) {
    using namespace OptionsUiKit;

    if (!state.staticDataReady) {
        DisabledGateText("Raid selection loads after static data is ready.");
        return;
    }

    std::lock_guard lock(state.dataMutex);

    SectionHeading("Raid Wing Selection");
    SectionSubtext("Choose which raid wings and encounters appear on the raid panel.");

    for (const auto& exp : state.raidData.expansions) {
        ImGui::PushID(exp.id.c_str());
        RenderExpansionBanner(exp.asset.c_str(), state);

        bool expansionVisible = state.raidVisibility.IsExpansionVisible(exp.id);
        if (ImGui::Checkbox(exp.name.c_str(), &expansionVisible)) {
            state.raidVisibility.SetExpansionVisible(exp.id, expansionVisible, state.raidData);
            SaveRaidVisibility(state);
        }

        ImGui::Indent(16.0f);
        for (const auto& wing : exp.wings) {
            ImGui::PushID(wing.id.c_str());

            bool wingVisible = state.raidVisibility.IsWingVisible(wing.id);
            const std::string wingLabel =
                wing.abbreviation.empty() ? wing.name : wing.abbreviation + " - " + wing.name;
            if (ImGui::Checkbox(wingLabel.c_str(), &wingVisible)) {
                state.raidVisibility.SetWingVisible(wing.id, wingVisible);
                SaveRaidVisibility(state);
            }

            ImGui::Indent(16.0f);
            for (const auto& enc : wing.encounters) {
                ImGui::PushID(enc.EncounterId().c_str());

                bool encounterVisible =
                    state.raidVisibility.IsEncounterVisible(enc.EncounterId());
                const std::string encounterLabel = enc.abbreviation.empty()
                                                       ? enc.name
                                                       : enc.abbreviation + " - " + enc.name;
                if (ImGui::Checkbox(encounterLabel.c_str(), &encounterVisible)) {
                    state.raidVisibility.SetEncounterVisible(enc.EncounterId(), encounterVisible);
                    SaveRaidVisibility(state);
                }

                ImGui::PopID();
            }
            ImGui::Unindent(16.0f);

            ImGui::PopID();
        }
        ImGui::Unindent(16.0f);

        ImGui::Separator();
        ImGui::PopID();
    }
}

void RenderStrikeSelection(AppState& state) {
    using namespace OptionsUiKit;

    if (!state.staticDataReady) {
        DisabledGateText("Raid encounter selection loads after static data is ready.");
        return;
    }

    std::lock_guard lock(state.dataMutex);

    SectionHeading("Raid Encounter Selection");
    SectionSubtext("Choose which raid encounters appear on the raid encounters panel.");

    if (state.dailyBountyData.enabled) {
        ImGui::TextColored(GoldColor(), "Daily bounties");
        bool priorityVisible = state.strikeVisibility.IsPriorityVisible();
        if (ImGui::Checkbox("Daily raid encounter bounties", &priorityVisible)) {
            state.strikeVisibility.SetPriorityVisible(priorityVisible);
            SaveStrikeVisibility(state);
        }

        bool tomorrowVisible = state.strikeVisibility.IsTomorrowBountiesVisible();
        if (ImGui::Checkbox("Tomorrow's raid encounter bounties", &tomorrowVisible)) {
            state.strikeVisibility.SetTomorrowBountiesVisible(tomorrowVisible);
            SaveStrikeVisibility(state);
        }

        ImGui::Separator();
    }

    for (const auto& exp : state.strikeData.expansions) {
        ImGui::PushID(exp.id.c_str());
        RenderExpansionBanner(exp.asset.c_str(), state);

        bool expansionVisible = state.strikeVisibility.IsExpansionVisible(exp.id);
        const std::string expansionLabel =
            exp.abbreviation.empty() ? exp.name : exp.abbreviation + " - " + exp.name;
        if (ImGui::Checkbox(expansionLabel.c_str(), &expansionVisible)) {
            state.strikeVisibility.SetExpansionVisible(exp.id, expansionVisible);
            SaveStrikeVisibility(state);
        }

        ImGui::Indent(16.0f);
        for (const auto& mission : exp.missions) {
            ImGui::PushID(mission.id.c_str());

            bool missionVisible = state.strikeVisibility.IsMissionVisible(mission.id);
            const std::string missionLabel = mission.abbreviation.empty()
                                                 ? mission.name
                                                 : mission.abbreviation + " - " + mission.name;
            if (ImGui::Checkbox(missionLabel.c_str(), &missionVisible)) {
                state.strikeVisibility.SetMissionVisible(mission.id, missionVisible);
                SaveStrikeVisibility(state);
            }

            ImGui::PopID();
        }
        ImGui::Unindent(16.0f);

        ImGui::Separator();
        ImGui::PopID();
    }
}

}  // namespace EncounterSelectionPanel
}  // namespace rc
