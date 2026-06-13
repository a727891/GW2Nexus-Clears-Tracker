#include "core/AppState.h"

#include "data/StaticDataLoader.h"
#include "services/DailyBountyService.h"
#include "services/RaidClearsService.h"
#include "services/StrikeClearsService.h"
#include "ui/UiFontService.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <thread>

namespace rc {

AppState& AppState::Instance() {
    static AppState state;
    return state;
}

void AppState::Initialize(AddonAPI_t* apiPtr) {
    api = apiPtr;
    nexusLink = static_cast<NexusLinkData_t*>(api->DataLink_Get(DL_NEXUS_LINK));
    mumbleLink = static_cast<Mumble::Data*>(api->DataLink_Get(DL_MUMBLE_LINK));

    addonDir = api->Paths_GetAddonDirectory("NexusRaidClears");
    const auto settingsPath = (std::filesystem::path(addonDir) / "settings.json").string();
    settings.Load(settingsPath);
    gw2Api.SetApiKey(settings.apiKey);

    rc::UiFontService::Initialize(api, nexusLink);

    apiPoll.SetIntervalMinutes(settings.pollIntervalMinutes);
    apiPoll.SetCallback([this]() { RequestApiRefresh(); });

    const auto strikePersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "strike_clears.json").string();

    mapWatcher.Initialize(&strikeData, &strikePersist, &resets, [this]() { return accountName; });
    mapWatcher.SetClearsCallback([this](const std::vector<std::string>& cleared) {
        std::lock_guard lock(dataMutex);
        strikeClearsSet.clear();
        for (const auto& id : cleared) strikeClearsSet.insert(id);
        ApplyStrikeClears();
        ApplyNonWeeklyHighlights();
    });

    trackedDailyReset_ = resets.LastDailyReset();
    trackedWeeklyReset_ = resets.LastWeeklyReset();

    if (LoadStaticDataFromCache()) {
        if (api) api->Log(LOGL_INFO, "NexusRaidClears", "Loaded static data from cache.");
    } else {
        if (api) {
            api->Log(LOGL_WARNING, "NexusRaidClears",
                      "Static data cache missing. Will download in background.");
        }
        RequestStaticDataLoad();
    }

    strikePersist.Load(strikePersistPath, strikeData);
    if (staticDataReady) {
        RequestApiRefresh();
    }
}

void AppState::Shutdown() {
    if (api) {
        rc::UiFontService::Shutdown(api);
    }
    const auto settingsPath = (std::filesystem::path(addonDir) / "settings.json").string();
    settings.Save(settingsPath);
    const auto strikePersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "strike_clears.json").string();
    strikePersist.Save(strikePersistPath);
}

void AppState::LoadStaticDataWithNetwork() {
    std::string raidJson, strikeJson, bountyJson;
    const bool ok = StaticDataLoader::Download(addonDir, "raid_data.json", raidJson) &&
                    StaticDataLoader::Download(addonDir, "strike_data.json", strikeJson) &&
                    StaticDataLoader::Download(addonDir, "daily_bounties.json", bountyJson);

    if (!ok) {
        if (api) {
            api->Log(LOGL_WARNING, "NexusRaidClears",
                      "Failed to download static JSON data.");
        }
        return;
    }

    try {
        std::lock_guard lock(dataMutex);
        raidData = RaidData::FromJson(nlohmann::json::parse(raidJson));
        strikeData = StrikeData::FromJson(nlohmann::json::parse(strikeJson));
        dailyBountyData = DailyBountyData::FromJson(nlohmann::json::parse(bountyJson));
        staticDataReady = true;
        weeklyBountyEncounters.Rebuild(dailyBountyData, resets);
        RebuildRaidGroups();
        RebuildStrikeGroups();
        if (api) api->Log(LOGL_INFO, "NexusRaidClears", "Downloaded static data.");
    } catch (...) {
        if (api) api->Log(LOGL_WARNING, "NexusRaidClears", "Failed to parse static JSON data.");
    }
}

bool AppState::LoadStaticDataFromCache() {
    std::string raidJson, strikeJson, bountyJson;
    const bool ok = StaticDataLoader::LoadCached(addonDir, "raid_data.json", raidJson) &&
                    StaticDataLoader::LoadCached(addonDir, "strike_data.json", strikeJson) &&
                    StaticDataLoader::LoadCached(addonDir, "daily_bounties.json", bountyJson);
    if (!ok) return false;

    try {
        raidData = RaidData::FromJson(nlohmann::json::parse(raidJson));
        strikeData = StrikeData::FromJson(nlohmann::json::parse(strikeJson));
        dailyBountyData = DailyBountyData::FromJson(nlohmann::json::parse(bountyJson));
        staticDataReady = true;
        weeklyBountyEncounters.Rebuild(dailyBountyData, resets);
        RebuildRaidGroups();
        RebuildStrikeGroups();
        return true;
    } catch (...) {
        if (api) api->Log(LOGL_WARNING, "NexusRaidClears", "Failed to parse cached static JSON.");
        staticDataReady = false;
        return false;
    }
}

void AppState::RequestStaticDataLoad() { staticDataLoadPending.store(true); }

void AppState::ProcessPendingStaticDataLoad() {
    if (!staticDataLoadPending.exchange(false)) return;

    std::thread([this]() {
        LoadStaticDataWithNetwork();
        if (staticDataReady) {
            RequestApiRefresh();
        }
    }).detach();
}

void AppState::RebuildRaidGroups() {
    raidGroups.clear();
    for (const auto& exp : raidData.expansions) {
        for (const auto& wing : exp.wings) {
            GridGroup group;
            group.id = wing.id;
            group.name = wing.name;
            group.abbreviation = wing.abbreviation;
            for (const auto& enc : wing.encounters) {
                EncounterCell cell;
                cell.id = enc.EncounterId();
                cell.name = enc.name;
                cell.abbreviation = enc.abbreviation;
                cell.state = ClearState::Unknown;
                if (enc.dailyBountyAchievementId) {
                    cell.dailyBountyAchievementId = *enc.dailyBountyAchievementId;
                }
                group.encounters.push_back(std::move(cell));
            }
            raidGroups.push_back(std::move(group));
        }
    }
}

void AppState::RebuildStrikeGroups() {
    strikeGroups.clear();
    for (const auto& exp : strikeData.expansions) {
        GridGroup group;
        group.id = exp.id;
        group.name = exp.name;
        group.abbreviation = exp.abbreviation;
        for (const auto& m : exp.missions) {
            EncounterCell cell;
            cell.id = m.id;
            cell.name = m.name;
            cell.abbreviation = m.abbreviation;
            cell.state = ClearState::Unknown;
            group.encounters.push_back(std::move(cell));
        }
        strikeGroups.push_back(std::move(group));
    }

    if (dailyBountyData.enabled) {
        GridGroup daily;
        daily.id = strikeData.priority.id.empty() ? "priority" : strikeData.priority.id;
        daily.name = strikeData.priority.name.empty() ? dailyBountyData.name : strikeData.priority.name;
        daily.abbreviation = strikeData.priority.abbreviation.empty()
                                   ? dailyBountyData.abbreviation
                                   : strikeData.priority.abbreviation;
        daily.isDailyBounty = true;
        daily.encounters =
            DailyBountyService::GetDailyBounties(dailyBountyData, raidData, strikeData, 0);
        strikeGroups.push_back(std::move(daily));

        GridGroup tomorrow;
        tomorrow.id = strikeData.priorityTomorrow.id.empty() ? "priority_tomorrow"
                                                             : strikeData.priorityTomorrow.id;
        tomorrow.name = strikeData.priorityTomorrow.name.empty() ? dailyBountyData.tomorrowName
                                                                   : strikeData.priorityTomorrow.name;
        tomorrow.abbreviation =
            strikeData.priorityTomorrow.abbreviation.empty()
                ? dailyBountyData.tomorrowAbbreviation
                : strikeData.priorityTomorrow.abbreviation;
        tomorrow.isTomorrowBounty = true;
        tomorrow.encounters =
            DailyBountyService::GetDailyBounties(dailyBountyData, raidData, strikeData, 1);
        strikeGroups.push_back(std::move(tomorrow));
    }
}

void AppState::RequestApiRefresh() {
    apiRefreshPending.store(true);
}

void AppState::ProcessPendingApiRefresh() {
    if (!apiRefreshPending.exchange(false)) return;

    std::thread([this]() {
        OnApiPoll();
    }).detach();
}

void AppState::OnApiPoll() {
    if (settings.apiKey.empty()) return;

    if (auto name = gw2Api.FetchAccountName()) {
        accountName = *name;
    }

    if (auto clears = gw2Api.FetchRaidClears()) {
        std::lock_guard lock(dataMutex);
        raidClearsSet = std::move(*clears);
        ApplyRaidClears();
        ApplyNonWeeklyHighlights();
    }

    StrikeClearsService::RefreshFromApi(strikeData, accountName, gw2Api, strikePersist);
    const auto strikePersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "strike_clears.json").string();
    strikePersist.Save(strikePersistPath);
    mapWatcher.DispatchCurrentStrikeClears();

    if (dailyBountyData.enabled) {
        dailyBountyProgress.RefreshFromApi(gw2Api, dailyBountyData.dailyBountyCategoryUrl);
        std::lock_guard lock(dataMutex);
        completedBountyAchievementIds = dailyBountyProgress.GetCompletedIds();
        ApplyDailyBountyClears();
    }
}

void AppState::ApplyRaidClears() {
    RaidClearsService::ApplyClears(raidGroups, raidClearsSet);
}

void AppState::ApplyNonWeeklyHighlights() {
    for (auto& group : raidGroups) {
        for (auto& enc : group.encounters) {
            enc.highlightNonWeeklyBounty = false;
            if (enc.state == ClearState::Cleared) continue;
            if (!settings.highlightNonWeeklyBounty) continue;
            if (settings.omitEventEncounters && raidData.IsEventEncounter(enc.id)) continue;
            if (!weeklyBountyEncounters.IsWeeklyBounty(enc.id)) {
                enc.highlightNonWeeklyBounty = true;
            }
        }
    }

    for (auto& group : strikeGroups) {
        if (group.isDailyBounty || group.isTomorrowBounty) continue;
        for (auto& enc : group.encounters) {
            enc.highlightNonWeeklyBounty = false;
            if (enc.state == ClearState::Cleared) continue;
            if (!settings.highlightNonWeeklyBounty) continue;
            if (!weeklyBountyEncounters.IsWeeklyBounty(enc.id)) {
                enc.highlightNonWeeklyBounty = true;
            }
        }
    }
}

void AppState::ApplyStrikeClears() {
    for (auto& group : strikeGroups) {
        if (group.isDailyBounty || group.isTomorrowBounty) continue;
        for (auto& enc : group.encounters) {
            if (strikeClearsSet.count(enc.id) > 0) {
                enc.state = ClearState::Cleared;
            } else {
                enc.state = ClearState::NotCleared;
            }
        }
    }
}

void AppState::ApplyDailyBountyClears() {
    for (auto& group : strikeGroups) {
        if (!group.isDailyBounty) continue;
        for (auto& enc : group.encounters) {
            if (enc.dailyBountyAchievementId > 0 &&
                completedBountyAchievementIds.count(enc.dailyBountyAchievementId) > 0) {
                enc.state = ClearState::Cleared;
            } else {
                enc.state = ClearState::NotCleared;
            }
        }
    }
}

bool AppState::ShouldShowPanel(bool panelVisible) const {
    if (!panelVisible) return false;
    if (nexusLink && !nexusLink->IsGameplay) return false;
    if (Mumble::IsMapOpen(mumbleLink)) return false;
    return true;
}

void AppState::TickResets() {
    resets.Update();
    if (resets.LastWeeklyReset() != trackedWeeklyReset_) {
        trackedWeeklyReset_ = resets.LastWeeklyReset();
        weeklyBountyEncounters.Rebuild(dailyBountyData, resets);
        std::lock_guard lock(dataMutex);
        ApplyNonWeeklyHighlights();
    }
    if (resets.LastDailyReset() != trackedDailyReset_) {
        trackedDailyReset_ = resets.LastDailyReset();
        weeklyBountyEncounters.Rebuild(dailyBountyData, resets);
        std::lock_guard lock(dataMutex);
        RebuildStrikeGroups();
        ApplyStrikeClears();
        ApplyDailyBountyClears();
        ApplyNonWeeklyHighlights();
    }
}

}  // namespace rc
