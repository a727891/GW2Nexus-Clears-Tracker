#include "services/FractalRotationService.h"

#include "services/FractalPersistance.h"
#include "services/PriorityRotationService.h"

#include <chrono>

namespace rc {
namespace {

constexpr int kDailyRotationMaxIndex = 15;

EncounterCell MakeCell(const FractalMap& map,
                       const FractalPersistance* labels,
                       const std::string& nameOverride = {}) {
    EncounterCell cell;
    cell.id = map.apiLabel;
    cell.name = nameOverride.empty() ? map.label : nameOverride;
    cell.abbreviation = labels ? labels->GetEncounterLabel(map.apiLabel, map.shortLabel)
                               : map.shortLabel;
    cell.state = ClearState::Unknown;
    return cell;
}

GridGroup MakeGroup(const std::string& id,
                    const std::string& name,
                    const std::string& abbreviation,
                    std::vector<EncounterCell> encounters,
                    const FractalPersistance* labels,
                    bool isTomorrowFractal = false) {
    GridGroup group;
    group.id = id;
    group.name = name;
    group.abbreviation =
        labels ? labels->GetEncounterLabel(id, abbreviation) : abbreviation;
    group.encounters = std::move(encounters);
    group.isTomorrowFractal = isTomorrowFractal;
    return group;
}

std::vector<FractalMap> DailyTierRotation(const FractalMapData& data, int index) {
    std::vector<FractalMap> result;
    if (index < 0 || index >= static_cast<int>(data.dailyTier.size())) {
        return result;
    }
    for (const auto& fractalName : data.dailyTier[index]) {
        result.push_back(data.GetFractalByName(fractalName));
    }
    return result;
}

std::vector<int> DailyRecsRotation(const FractalMapData& data, int index) {
    if (index >= 0 && index < static_cast<int>(data.recs.size())) {
        return data.recs[index];
    }
    return {96, 97, 98, 99, 100};
}

}  // namespace

std::vector<GridGroup> FractalRotationService::BuildGroups(const FractalMapData& data,
                                                           const SettingsStore& settings,
                                                           const FractalPersistance* labels) {
    std::vector<GridGroup> groups;
    const int dayIndex = DayOfYearIndex(std::chrono::system_clock::now());
    const int today = dayIndex % kDailyRotationMaxIndex;
    const int tomorrow = (today + 1) % kDailyRotationMaxIndex;

    if (settings.fractalChallengeMotes) {
        std::vector<EncounterCell> cells;
        for (const int scale : data.challengeMotes) {
            const auto map = data.GetFractalForScale(scale);
            if (map.IsValid()) {
                cells.push_back(MakeCell(map, labels));
            }
        }
        groups.push_back(MakeGroup("CM", "Challenge Mote", "CM", std::move(cells), labels));
    }

    if (settings.fractalTomorrowTierN) {
        std::vector<EncounterCell> cells;
        for (const auto& map : DailyTierRotation(data, tomorrow)) {
            if (map.IsValid()) {
                cells.push_back(MakeCell(map, labels));
            }
        }
        groups.push_back(
            MakeGroup("Tom", "Tomorrow T#", "Tom", std::move(cells), labels, true));
    }

    if (settings.fractalDailyTierN) {
        const auto todayMaps = DailyTierRotation(data, today);
        const auto tomorrowMaps = DailyTierRotation(data, tomorrow);
        std::vector<EncounterCell> cells;
        const size_t count = std::max(todayMaps.size(), tomorrowMaps.size());
        for (size_t i = 0; i < count; ++i) {
            const auto& todayMap = i < todayMaps.size() ? todayMaps[i] : FractalMap{};
            const auto& tomorrowMap =
                i < tomorrowMaps.size() ? tomorrowMaps[i] : FractalMap{};
            if (!todayMap.IsValid()) continue;
            std::string tooltipName = todayMap.label;
            if (tomorrowMap.IsValid()) {
                tooltipName += "\n\nTomorrow:\n" + tomorrowMap.label;
            }
            cells.push_back(MakeCell(todayMap, labels, tooltipName));
        }
        groups.push_back(MakeGroup("T#", "Tier #", "T#", std::move(cells), labels));
    }

    if (settings.fractalDailyRecs) {
        const auto todayScales = DailyRecsRotation(data, today);
        const auto tomorrowScales = DailyRecsRotation(data, tomorrow);
        std::vector<EncounterCell> cells;
        const size_t count = std::max(todayScales.size(), tomorrowScales.size());
        for (size_t i = 0; i < count; ++i) {
            const auto todayMap =
                i < todayScales.size() ? data.GetFractalForScale(todayScales[i]) : FractalMap{};
            const auto tomorrowMap = i < tomorrowScales.size()
                                         ? data.GetFractalForScale(tomorrowScales[i])
                                         : FractalMap{};
            if (!todayMap.IsValid()) continue;
            std::string tooltipName = todayMap.label;
            if (tomorrowMap.IsValid()) {
                tooltipName += "\n\nTomorrow:\n" + tomorrowMap.label;
            }
            cells.push_back(MakeCell(todayMap, labels, tooltipName));
        }
        groups.push_back(MakeGroup("Rec", "Daily Recommended", "Rec", std::move(cells), labels));
    }

    return groups;
}

}  // namespace rc
