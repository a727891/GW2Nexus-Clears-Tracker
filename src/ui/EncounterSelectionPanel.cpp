#include "ui/EncounterSelectionPanel.h"

#include "core/AppState.h"

#include <imgui.h>
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

void RenderRaidSelection(AppState& state) {
    if (!ImGui::CollapsingHeader("Raid wings")) return;

    for (const auto& exp : state.raidData.expansions) {
        ImGui::PushID(exp.id.c_str());

        bool expansionVisible = state.raidVisibility.IsExpansionVisible(exp.id);
        if (ImGui::Checkbox(exp.name.c_str(), &expansionVisible)) {
            state.raidVisibility.SetExpansionVisible(exp.id, expansionVisible, state.raidData);
            SaveRaidVisibility(state);
        }

        if (ImGui::TreeNode("Wings")) {
            for (const auto& wing : exp.wings) {
                ImGui::PushID(wing.id.c_str());

                bool wingVisible = state.raidVisibility.IsWingVisible(wing.id);
                const std::string wingLabel =
                    wing.abbreviation.empty() ? wing.name : wing.abbreviation + " - " + wing.name;
                if (ImGui::Checkbox(wingLabel.c_str(), &wingVisible)) {
                    state.raidVisibility.SetWingVisible(wing.id, wingVisible);
                    SaveRaidVisibility(state);
                }

                if (ImGui::TreeNode("Encounters")) {
                    for (const auto& enc : wing.encounters) {
                        ImGui::PushID(enc.EncounterId().c_str());

                        bool encounterVisible =
                            state.raidVisibility.IsEncounterVisible(enc.EncounterId());
                        const std::string encounterLabel = enc.abbreviation.empty()
                                                               ? enc.name
                                                               : enc.abbreviation + " - " + enc.name;
                        if (ImGui::Checkbox(encounterLabel.c_str(), &encounterVisible)) {
                            state.raidVisibility.SetEncounterVisible(enc.EncounterId(),
                                                                     encounterVisible);
                            SaveRaidVisibility(state);
                        }

                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        ImGui::Separator();
        ImGui::PopID();
    }
}

void RenderStrikeSelection(AppState& state) {
    if (!ImGui::CollapsingHeader("Raid encounters")) return;

    if (state.dailyBountyData.enabled) {
        ImGui::Text("Daily bounties");
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

        bool expansionVisible = state.strikeVisibility.IsExpansionVisible(exp.id);
        const std::string expansionLabel =
            exp.abbreviation.empty() ? exp.name : exp.abbreviation + " - " + exp.name;
        if (ImGui::Checkbox(expansionLabel.c_str(), &expansionVisible)) {
            state.strikeVisibility.SetExpansionVisible(exp.id, expansionVisible);
            SaveStrikeVisibility(state);
        }

        if (ImGui::TreeNode("Raid encounters")) {
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
            ImGui::TreePop();
        }

        ImGui::Separator();
        ImGui::PopID();
    }
}

}  // namespace

void Render(AppState& state) {
    if (!state.staticDataReady) {
        ImGui::TextDisabled("Encounter selection loads after static data is ready.");
        return;
    }

    std::lock_guard lock(state.dataMutex);
    RenderRaidSelection(state);
    RenderStrikeSelection(state);
}

}  // namespace EncounterSelectionPanel
}  // namespace rc
