#include "core/AppState.h"

#include "data/DungeonData.h"
#include "data/StaticDataLoader.h"
#include "services/DailyBountyService.h"
#include "services/DatAssetIconService.h"
#include "services/DungeonsClearsService.h"
#include "services/FractalRotationService.h"
#include "services/RaidClearsService.h"
#include "services/StrikeClearsService.h"
#include "ui/GridMaskService.h"
#include "ui/QuickAccessService.h"
#include "ui/UiFontService.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <thread>

namespace rc {

namespace {

bool LoadFractalMapData(const std::string& addonDir, FractalMapData& outData) {
    std::string fractalJson;
    if (!StaticDataLoader::LoadOrDownload(addonDir, "fractal_maps.json", fractalJson)) {
        return false;
    }
    try {
        outData = FractalMapData::FromJson(nlohmann::json::parse(fractalJson));
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace

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

    raidVisibility.Load((std::filesystem::path(addonDir) / "raid_settings.json").string());
    strikeVisibility.Load((std::filesystem::path(addonDir) / "strike_settings.json").string());

    rc::UiFontService::Initialize(api, nexusLink);
    rc::GridMaskService::Initialize(api, addonDir);
    rc::DatAssetIconService::Initialize(api, addonDir);

    const auto mentorCachePath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "mentor_achievement_progress.json")
            .string();
    mentorProgress.Initialize(raidData, mentorCachePath);
    mentorProgress.LoadCache();

    apiPoll.SetIntervalMinutes(settings.pollIntervalMinutes);
    apiPoll.SetCallback([this]() { RequestApiRefresh(); });

    const auto strikePersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "strike_clears.json").string();
    const auto fractalPersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "fractal_clears.json").string();

    mapWatcher.Initialize(&strikeData, &strikePersist, &resets, [this]() { return accountName; });
    mapWatcher.SetClearsCallback([this](const std::vector<std::string>& cleared) {
        std::lock_guard lock(dataMutex);
        strikeClearsSet.clear();
        for (const auto& id : cleared) strikeClearsSet.insert(id);
        ApplyStrikeClears();
        ApplyNonWeeklyHighlights();
    });

    fractalMapWatcher.Initialize(&fractalMapData, &fractalPersist, &resets,
                                 [this]() { return accountName; });
    fractalMapWatcher.SetClearsCallback([this](const std::vector<std::string>& cleared) {
        std::lock_guard lock(dataMutex);
        fractalClearsSet.clear();
        for (const auto& id : cleared) fractalClearsSet.insert(id);
        ApplyFractalClears();
        const auto fractalPersistPath =
            (std::filesystem::path(addonDir) / "clearsTracker" / "fractal_clears.json").string();
        fractalPersist.Save(fractalPersistPath);
    });

    RebuildDungeonGroups();

    trackedDailyReset_ = resets.LastDailyReset();
    trackedWeeklyReset_ = resets.LastWeeklyReset();

    if (LoadStaticDataFromCache()) {
        if (api) api->Log(LOGL_INFO, "NexusRaidClears", "Loaded static data from cache.");
        LoadClearsTrackerMetadata();
        RefreshTooltipServices();
    } else {
        if (api) {
            api->Log(LOGL_WARNING, "NexusRaidClears",
                      "Static data cache missing. Will download in background.");
        }
        RequestStaticDataLoad();
    }

    if (LoadFractalMapData(addonDir, fractalMapData)) {
        fractalDataReady = true;
        RebuildFractalGroups();
        if (api) api->Log(LOGL_INFO, "NexusRaidClears", "Loaded fractal map data.");
    } else if (api) {
        api->Log(LOGL_WARNING, "NexusRaidClears",
                  "Fractal map data missing. Will download in background.");
        RequestStaticDataLoad();
    }

    strikePersist.Load(strikePersistPath, strikeData);
    if (fractalDataReady) {
        fractalPersist.Load(fractalPersistPath, fractalMapData);
        fractalMapWatcher.DispatchCurrentFractalClears();
    }
    if (staticDataReady) {
        RequestApiRefresh();
    }
}

void AppState::Shutdown() {
    if (api) {
        rc::UiFontService::Shutdown(api);
        rc::DatAssetIconService::Shutdown();
    }
    const auto settingsPath = (std::filesystem::path(addonDir) / "settings.json").string();
    settings.Save(settingsPath);
    raidVisibility.Save((std::filesystem::path(addonDir) / "raid_settings.json").string());
    strikeVisibility.Save((std::filesystem::path(addonDir) / "strike_settings.json").string());
    const auto strikePersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "strike_clears.json").string();
    const auto fractalPersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "fractal_clears.json").string();
    strikePersist.Save(strikePersistPath);
    fractalPersist.Save(fractalPersistPath);
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
        SyncEncounterVisibility();
        LoadClearsTrackerMetadata();
        RefreshTooltipServices();
        if (!fractalDataReady && LoadFractalMapData(addonDir, fractalMapData)) {
            fractalDataReady = true;
            RebuildFractalGroups();
            const auto fractalPersistPath =
                (std::filesystem::path(addonDir) / "clearsTracker" / "fractal_clears.json")
                    .string();
            fractalPersist.Load(fractalPersistPath, fractalMapData);
            fractalMapWatcher.DispatchCurrentFractalClears();
        }
        if (api) {
            QuickAccessService::Refresh(api, *this);
            api->Log(LOGL_INFO, "NexusRaidClears", "Downloaded static data.");
        }
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
        SyncEncounterVisibility();
        LoadClearsTrackerMetadata();
        RefreshTooltipServices();
        if (!fractalDataReady && LoadFractalMapData(addonDir, fractalMapData)) {
            fractalDataReady = true;
            RebuildFractalGroups();
        }
        return true;
    } catch (...) {
        if (api) api->Log(LOGL_WARNING, "NexusRaidClears", "Failed to parse cached static JSON.");
        staticDataReady = false;
        return false;
    }
}

void AppState::RequestStaticDataLoad() { staticDataLoadPending.store(true); }

bool AppState::LoadClearsTrackerMetadata() {
    std::string trackerJson;
    if (!StaticDataLoader::LoadOrDownload(addonDir, "clears_tracker.json", trackerJson)) {
        return false;
    }

    try {
        const auto j = nlohmann::json::parse(trackerJson);
        if (j.contains("motd") && j["motd"].is_string()) {
            motd = j["motd"].get<std::string>();
        } else {
            motd.clear();
        }
        if (j.contains("motd_id") && j["motd_id"].is_string()) {
            motdId = j["motd_id"].get<std::string>();
        } else {
            motdId.clear();
        }
        return true;
    } catch (...) {
        if (api) {
            api->Log(LOGL_WARNING, "NexusRaidClears", "Failed to parse clears_tracker.json.");
        }
        return false;
    }
}

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
        daily.name = "Daily Raid Encounter Bounties";
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
        tomorrow.name = "Tomorrow's Raid Encounter Bounties";
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

void AppState::RebuildFractalGroups() {
    fractalGroups = FractalRotationService::BuildGroups(fractalMapData, settings);
    ApplyFractalClears();
}

void AppState::RebuildDungeonGroups() {
    dungeonGroups.clear();
    for (const auto& def : DungeonData::Groups()) {
        GridGroup group;
        group.id = def.abbreviation;
        group.name = def.name;
        group.abbreviation = def.abbreviation;
        group.isFrequenterSummary = def.isFrequenterSummary;
        for (const auto& path : def.paths) {
            EncounterCell cell;
            cell.id = path.id;
            cell.name = path.name;
            cell.abbreviation = path.abbreviation;
            cell.state = ClearState::Unknown;
            group.encounters.push_back(std::move(cell));
        }
        dungeonGroups.push_back(std::move(group));
    }
    ApplyDungeonClears();
}

void AppState::SyncEncounterVisibility() {
    raidVisibility.InitializeFromData(raidData);
    strikeVisibility.InitializeFromData(strikeData);
}

void AppState::RefreshTooltipServices() {
    const auto mentorCachePath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "mentor_achievement_progress.json")
            .string();
    mentorProgress.Initialize(raidData, mentorCachePath);
    DatAssetIconService::PreloadIndicators(raidData.powerDamageAssetId, raidData.condiDamageAssetId,
                                           raidData.defianceAssetId, raidData.mentorAssetId);
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
        if (api) QuickAccessService::Refresh(api, *this);
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

    mentorProgress.RefreshFromApi(gw2Api, settings.showMentorProgress);

    if (auto dungeonClears = gw2Api.FetchDungeonClears()) {
        std::lock_guard lock(dataMutex);
        dungeonClearsSet = std::move(*dungeonClears);
    }
    if (auto frequenterPaths = gw2Api.FetchFrequenterPaths()) {
        std::lock_guard lock(dataMutex);
        frequenterPathsSet = std::move(*frequenterPaths);
    }
    {
        std::lock_guard lock(dataMutex);
        ApplyDungeonClears();
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

void AppState::ApplyFractalClears() {
    for (auto& group : fractalGroups) {
        if (group.isTomorrowFractal) continue;
        for (auto& enc : group.encounters) {
            if (fractalClearsSet.count(enc.id) > 0) {
                enc.state = ClearState::Cleared;
            } else {
                enc.state = ClearState::NotCleared;
            }
        }
    }
}

void AppState::ApplyDungeonClears() {
    for (auto& group : dungeonGroups) {
        for (auto& enc : group.encounters) {
            enc.highlightFrequenter = frequenterPathsSet.count(enc.id) > 0;
            if (group.isFrequenterSummary && enc.id == DungeonData::kFrequenterId) {
                enc.abbreviation = std::to_string(frequenterPathsSet.size()) + "/8";
                enc.state = ClearState::Unknown;
                continue;
            }
            if (dungeonClearsSet.count(enc.id) > 0) {
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
        if (fractalDataReady) {
            RebuildFractalGroups();
            fractalMapWatcher.DispatchCurrentFractalClears();
        }
        ApplyStrikeClears();
        ApplyDailyBountyClears();
        ApplyNonWeeklyHighlights();
    }
}

}  // namespace rc
