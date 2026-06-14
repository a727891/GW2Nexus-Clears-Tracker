#include "ui/LabelCustomizationPanel.h"

#include "core/AppState.h"

#include <cstring>
#include <imgui.h>
#include <mutex>
#include <string>

namespace rc {
namespace LabelCustomizationPanel {
namespace {

constexpr int kLabelBufferSize = 48;

enum class LabelStore { Raid, Strike, Fractal };

ImVec4 ColorFromRgb(uint8_t r, uint8_t g, uint8_t b) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void SaveRaidSettings(AppState& state) {
    const auto path = state.addonDir + "/raid_settings.json";
    state.raidVisibility.Save(path);
}

void SaveStrikeSettings(AppState& state) {
    const auto path = state.addonDir + "/strike_settings.json";
    state.strikeVisibility.Save(path);
}

void SaveFractalSettings(AppState& state) {
    const auto path = state.addonDir + "/clearsTracker/fractal_clears.json";
    state.fractalPersist.Save(path);
}

std::string GetLabel(const AppState& state, LabelStore store, const std::string& id,
                     const std::string& defaultAbbrev) {
    switch (store) {
        case LabelStore::Raid:
            return state.raidVisibility.GetEncounterLabel(id, defaultAbbrev);
        case LabelStore::Strike:
            return state.strikeVisibility.GetEncounterLabel(id, defaultAbbrev);
        case LabelStore::Fractal:
            return state.fractalPersist.GetEncounterLabel(id, defaultAbbrev);
    }
    return defaultAbbrev;
}

void SetLabel(AppState& state, LabelStore store, const std::string& id, const std::string& label) {
    switch (store) {
        case LabelStore::Raid:
            state.raidVisibility.SetEncounterLabel(id, label);
            SaveRaidSettings(state);
            break;
        case LabelStore::Strike:
            state.strikeVisibility.SetEncounterLabel(id, label);
            SaveStrikeSettings(state);
            break;
        case LabelStore::Fractal:
            state.fractalPersist.SetEncounterLabel(id, label);
            SaveFractalSettings(state);
            break;
    }
    state.ApplyEncounterLabel(id, label);
}

bool RenderLabelRow(AppState& state,
                    const std::string& id,
                    const std::string& name,
                    const std::string& defaultAbbrev,
                    LabelStore store,
                    const ImVec4& nameColor) {
    ImGui::PushID(id.c_str());

    const std::string currentLabel = GetLabel(state, store, id, defaultAbbrev);

    ImGui::TextColored(nameColor, "%s", name.c_str());
    ImGui::SameLine(220.0f);

    char buffer[kLabelBufferSize] = {};
    std::strncpy(buffer, currentLabel.c_str(), sizeof(buffer) - 1);

    bool changed = false;
    if (ImGui::InputText("##label", buffer, sizeof(buffer))) {
        changed = true;
    }

    if (currentLabel != defaultAbbrev) {
        ImGui::SameLine();
        const std::string resetLabel = "Reset to " + defaultAbbrev;
        if (ImGui::Button(resetLabel.c_str())) {
            SetLabel(state, store, id, defaultAbbrev);
            ImGui::PopID();
            return true;
        }
    }

    if (changed && std::string(buffer) != currentLabel) {
        SetLabel(state, store, id, buffer);
    }

    ImGui::PopID();
    return changed;
}

void RenderRaidLabels(AppState& state) {
    if (!ImGui::CollapsingHeader("Customize raid labels")) return;

    ImGui::TextWrapped(
        "Change the short labels shown on the raid panel. Labels are saved per account.");
    ImGui::Spacing();

    const ImVec4 gold = ColorFromRgb(255, 200, 0);
    const ImVec4 gray = ColorFromRgb(180, 180, 180);
    const ImVec4 white = ColorFromRgb(255, 255, 255);

    for (const auto& exp : state.raidData.expansions) {
        ImGui::TextColored(gold, "%s", exp.name.c_str());
        for (const auto& wing : exp.wings) {
            RenderLabelRow(state, wing.id, wing.name, wing.abbreviation, LabelStore::Raid, gray);
            for (const auto& enc : wing.encounters) {
                RenderLabelRow(state, enc.EncounterId(), enc.name, enc.abbreviation,
                               LabelStore::Raid, white);
            }
            ImGui::Spacing();
        }
        ImGui::Separator();
    }
}

void RenderStrikeLabels(AppState& state) {
    if (!ImGui::CollapsingHeader("Customize raid encounter labels")) return;

    ImGui::TextWrapped(
        "Change the short labels shown on the raid encounters panel. Labels are saved per "
        "account.");
    ImGui::Spacing();

    const ImVec4 gold = ColorFromRgb(255, 200, 0);
    const ImVec4 white = ColorFromRgb(255, 255, 255);

    if (state.dailyBountyData.enabled) {
        const std::string priorityId =
            state.strikeData.priority.id.empty() ? "priority" : state.strikeData.priority.id;
        const std::string priorityDefault =
            state.strikeData.priority.abbreviation.empty()
                ? state.dailyBountyData.abbreviation
                : state.strikeData.priority.abbreviation;
        RenderLabelRow(state, priorityId, "Daily Raid Encounter Bounties", priorityDefault,
                       LabelStore::Strike, gold);

        const std::string tomorrowId = state.strikeData.priorityTomorrow.id.empty()
                                           ? "priority_tomorrow"
                                           : state.strikeData.priorityTomorrow.id;
        const std::string tomorrowDefault =
            state.strikeData.priorityTomorrow.abbreviation.empty()
                ? state.dailyBountyData.tomorrowAbbreviation
                : state.strikeData.priorityTomorrow.abbreviation;
        RenderLabelRow(state, tomorrowId, "Tomorrow's Raid Encounter Bounties", tomorrowDefault,
                       LabelStore::Strike, gold);
        ImGui::Spacing();
    }

    for (const auto& exp : state.strikeData.expansions) {
        RenderLabelRow(state, exp.id, exp.name, exp.abbreviation, LabelStore::Strike, gold);
        for (const auto& mission : exp.missions) {
            RenderLabelRow(state, mission.id, mission.name, mission.abbreviation, LabelStore::Strike,
                           white);
        }
        ImGui::Spacing();
    }
}

void RenderFractalLabels(AppState& state) {
    if (!state.fractalDataReady) return;
    if (!ImGui::CollapsingHeader("Customize fractal labels")) return;

    ImGui::TextWrapped(
        "Change the short labels shown on the fractals panel. Labels are saved per account.");
    ImGui::Spacing();

    const ImVec4 gold = ColorFromRgb(255, 200, 0);
    const ImVec4 white = ColorFromRgb(255, 255, 255);

    static const struct {
        const char* id;
        const char* name;
        const char* abbrev;
    } kCategories[] = {
        {"CM", "Challenge Mote", "CM"},
        {"Tom", "Tomorrow T#", "Tom"},
        {"T#", "Tier #", "T#"},
        {"Rec", "Daily Recommended", "Rec"},
    };

    for (const auto& category : kCategories) {
        RenderLabelRow(state, category.id, category.name, category.abbrev, LabelStore::Fractal,
                       gold);
    }

    ImGui::Spacing();

    for (const auto& [_, map] : state.fractalMapData.maps) {
        if (!map.IsValid()) continue;
        RenderLabelRow(state, map.apiLabel, map.label, map.shortLabel, LabelStore::Fractal, white);
    }
}

}  // namespace

void Render(AppState& state) {
    if (!state.staticDataReady) {
        ImGui::TextDisabled("Label customization loads after static data is ready.");
        return;
    }

    std::lock_guard lock(state.dataMutex);
    RenderRaidLabels(state);
    RenderStrikeLabels(state);
    RenderFractalLabels(state);
}

}  // namespace LabelCustomizationPanel
}  // namespace rc
