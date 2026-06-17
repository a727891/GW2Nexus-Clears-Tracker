#include "ui/EncounterSelectionPanel.h"

#include "core/AppState.h"
#include "ui/OptionsUiKit.h"

#include <filesystem>
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
        BeginExpansionRow(exp.id.c_str());

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

        EndExpansionRow();
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
        BeginExpansionRow(exp.id.c_str());

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

        EndExpansionRow();
        ImGui::Separator();
        ImGui::PopID();
    }
}

void RenderFractalSelection(AppState& state) {
    using namespace OptionsUiKit;

    if (!state.fractalDataReady) {
        DisabledGateText("Fractal selection loads after fractal data is ready.");
        return;
    }

    SectionHeading("Fractal Selection");
    SectionSubtext("Choose which fractal rows appear on the panel.");

    BeginExpansionRow("core");

    if (SettingCheckbox("Show challenge motes", &state.settings.fractalChallengeMotes,
                        "Display CM fractals. Hover tooltip shows today's and tomorrow's "
                        "instabilities.")) {
        std::lock_guard lock(state.dataMutex);
        state.RebuildFractalGroups();
    }

    if (state.settings.fractalChallengeMotes) {
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
        std::lock_guard lock(state.dataMutex);
        state.RebuildFractalGroups();
    }
    if (SettingCheckbox("Show daily recommended fractals", &state.settings.fractalDailyRecs)) {
        std::lock_guard lock(state.dataMutex);
        state.RebuildFractalGroups();
    }
    if (SettingCheckbox("Show tomorrow tier fractals", &state.settings.fractalTomorrowTierN)) {
        std::lock_guard lock(state.dataMutex);
        state.RebuildFractalGroups();
    }

    EndExpansionRow();
}

void RenderDungeonSelection(AppState& state) {
    using namespace OptionsUiKit;

    SectionHeading("Dungeon Selection");
    SectionSubtext("Toggle individual dungeon columns on the panel.");

    BeginExpansionRow("core");

    static const char* kDungeonNames[] = {"AC", "CM", "TA", "SE", "CoF", "HW", "CoE", "Arah"};
    for (size_t i = 0; i < state.settings.dungeonVisible.size(); ++i) {
        ImGui::Checkbox(kDungeonNames[i], &state.settings.dungeonVisible[i]);
    }

    EndExpansionRow();
}

}  // namespace EncounterSelectionPanel
}  // namespace rc
