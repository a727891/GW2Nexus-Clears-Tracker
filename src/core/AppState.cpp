#include "core/AppState.h"
#include "core/MumbleIdentity.h"
#include "core/StorageKeyUtil.h"

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
#include "ui/MentorProgressPopupService.h"
#include "ui/UiFontService.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <thread>

namespace rc {

namespace {

bool LoadFractalMapDataFromCache(const std::string& addonDir, FractalMapData& outData) {
    std::string fractalJson;
    if (!StaticDataLoader::LoadCached(addonDir, "fractal_maps.json", fractalJson)) {
        return false;
    }
    try {
        outData = FractalMapData::FromJson(nlohmann::json::parse(fractalJson));
        return true;
    } catch (...) {
        return false;
    }
}

bool ParseClearsTrackerJson(const std::string& trackerJson, std::string& motd, std::string& motdId) {
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

    accountRegistry.Load(ApiAccountsPath());
    accountRegistry.SetStoragePath(ApiAccountsPath());
    const auto legacyKey = settings.ConsumeLegacyApiKey();
    if (!legacyKey.empty()) {
        settings.Save(settingsPath);
        if (accountRegistry.AccountsSnapshot().empty()) {
            const auto accountsPath = ApiAccountsPath();
            std::thread([this, legacyKey, accountsPath]() {
                Gw2ApiClient client;
                const auto result = accountRegistry.RegisterKey(client, legacyKey);
                if (result.success) {
                    accountRegistry.Save(accountsPath);
                    pendingCharacterResolve.store(true);
                    if (api) {
                        api->Log(LOGL_INFO, "NexusRaidClears",
                                  "Migrated legacy API key to registered accounts.");
                    }
                } else if (api) {
                    api->Log(LOGL_WARNING, "NexusRaidClears", result.message.c_str());
                }
            }).detach();
        }
    }

    const auto initialCharacter = MumbleIdentity::ParseIdentityName(mumbleLink);
    characterName = initialCharacter;
    if (accountRegistry.ResolveActiveAccountFromCache(initialCharacter)) {
        OnActiveAccountChanged();
    } else {
        accountName = accountRegistry.ActiveAccountName().value_or("");
        mentorProgress.SetActiveAccount(accountName);
        if (!initialCharacter.empty()) {
            RequestAccountResolve();
        }
    }

    raidVisibility.Load((std::filesystem::path(addonDir) / "raid_settings.json").string());
    strikeVisibility.Load((std::filesystem::path(addonDir) / "strike_settings.json").string());

    rc::UiFontService::Initialize(api, nexusLink);
    rc::GridMaskService::Initialize(api, addonDir);
    rc::DatAssetIconService::Initialize(api, addonDir);

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

    if (LoadFractalMapDataFromCache(addonDir, fractalMapData)) {
        fractalDataReady = true;
        RebuildFractalGroups();
        if (api) api->Log(LOGL_INFO, "NexusRaidClears", "Loaded fractal map data from cache.");
    } else if (api) {
        api->Log(LOGL_WARNING, "NexusRaidClears",
                  "Fractal map data cache missing. Will download in background.");
        RequestStaticDataLoad();
    }
    LoadInstabilitiesData();

    strikePersist.Load(strikePersistPath, strikeData);
    if (fractalDataReady) {
        fractalPersist.Load(fractalPersistPath, fractalMapData);
        fractalPersist.EnsureChallengeMoteDefaults(fractalMapData);
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
    accountRegistry.Save(ApiAccountsPath());
    mentorProgress.SaveCache();
    const auto strikePersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "strike_clears.json").string();
    const auto fractalPersistPath =
        (std::filesystem::path(addonDir) / "clearsTracker" / "fractal_clears.json").string();
    strikePersist.Save(strikePersistPath);
    fractalPersist.Save(fractalPersistPath);
}

void AppState::LoadStaticDataWithNetwork() {
    std::string raidJson, strikeJson, bountyJson, trackerJson, fractalJson;
    const bool ok = StaticDataLoader::Download(addonDir, "raid_data.json", raidJson) &&
                    StaticDataLoader::Download(addonDir, "strike_data.json", strikeJson) &&
                    StaticDataLoader::Download(addonDir, "daily_bounties.json", bountyJson);
    StaticDataLoader::Download(addonDir, "clears_tracker.json", trackerJson);
    StaticDataLoader::Download(addonDir, "fractal_maps.json", fractalJson);

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
        if (!trackerJson.empty()) {
            ParseClearsTrackerJson(trackerJson, motd, motdId);
        }
        RefreshTooltipServices();
        if (!fractalDataReady && !fractalJson.empty()) {
            fractalMapData = FractalMapData::FromJson(nlohmann::json::parse(fractalJson));
            fractalDataReady = true;
            RebuildFractalGroups();
            const auto fractalPersistPath =
                (std::filesystem::path(addonDir) / "clearsTracker" / "fractal_clears.json")
                    .string();
            fractalPersist.Load(fractalPersistPath, fractalMapData);
            fractalPersist.EnsureChallengeMoteDefaults(fractalMapData);
            fractalMapWatcher.DispatchCurrentFractalClears();
        }
        LoadInstabilitiesData();
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
        if (!fractalDataReady && LoadFractalMapDataFromCache(addonDir, fractalMapData)) {
            fractalDataReady = true;
            RebuildFractalGroups();
        }
        LoadInstabilitiesData();
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
    if (!StaticDataLoader::LoadCached(addonDir, "clears_tracker.json", trackerJson)) {
        return false;
    }

    if (!ParseClearsTrackerJson(trackerJson, motd, motdId)) {
        if (api) {
            api->Log(LOGL_WARNING, "NexusRaidClears", "Failed to parse clears_tracker.json.");
        }
        return false;
    }
    return true;
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
            group.abbreviation =
                raidVisibility.GetEncounterLabel(wing.id, wing.abbreviation);
            for (const auto& enc : wing.encounters) {
                EncounterCell cell;
                cell.id = enc.EncounterId();
                cell.name = enc.name;
                cell.abbreviation =
                    raidVisibility.GetEncounterLabel(enc.EncounterId(), enc.abbreviation);
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
        group.abbreviation =
            strikeVisibility.GetEncounterLabel(exp.id, exp.abbreviation);
        for (const auto& m : exp.missions) {
            EncounterCell cell;
            cell.id = m.id;
            cell.name = m.name;
            cell.abbreviation =
                strikeVisibility.GetEncounterLabel(m.id, m.abbreviation);
            cell.state = ClearState::Unknown;
            group.encounters.push_back(std::move(cell));
        }
        strikeGroups.push_back(std::move(group));
    }

    if (dailyBountyData.enabled) {
        const std::string priorityDefault =
            strikeData.priority.abbreviation.empty() ? dailyBountyData.abbreviation
                                                     : strikeData.priority.abbreviation;
        const std::string tomorrowDefault =
            strikeData.priorityTomorrow.abbreviation.empty()
                ? dailyBountyData.tomorrowAbbreviation
                : strikeData.priorityTomorrow.abbreviation;

        GridGroup daily;
        daily.id = strikeData.priority.id.empty() ? "priority" : strikeData.priority.id;
        daily.name = "Daily Raid Encounter Bounties";
        daily.abbreviation =
            strikeVisibility.GetEncounterLabel(daily.id, priorityDefault);
        daily.isDailyBounty = true;
        daily.encounters = DailyBountyService::GetDailyBounties(
            dailyBountyData, raidData, strikeData, 0, &raidVisibility, &strikeVisibility);
        strikeGroups.push_back(std::move(daily));

        GridGroup tomorrow;
        tomorrow.id = strikeData.priorityTomorrow.id.empty() ? "priority_tomorrow"
                                                             : strikeData.priorityTomorrow.id;
        tomorrow.name = "Tomorrow's Raid Encounter Bounties";
        tomorrow.abbreviation =
            strikeVisibility.GetEncounterLabel(tomorrow.id, tomorrowDefault);
        tomorrow.isTomorrowBounty = true;
        tomorrow.encounters = DailyBountyService::GetDailyBounties(
            dailyBountyData, raidData, strikeData, 1, &raidVisibility, &strikeVisibility);
        strikeGroups.push_back(std::move(tomorrow));
    }
}

void AppState::ApplyEncounterLabel(const std::string& encounterId, const std::string& label) {
    const auto normalized = NormalizeStorageKey(encounterId);
    const auto matches = [&](const std::string& id) {
        return id == encounterId || NormalizeStorageKey(id) == normalized;
    };

    for (auto& group : raidGroups) {
        if (matches(group.id)) {
            group.abbreviation = label;
        }
        for (auto& cell : group.encounters) {
            if (matches(cell.id)) {
                cell.abbreviation = label;
            }
        }
    }

    for (auto& group : strikeGroups) {
        if (matches(group.id)) {
            group.abbreviation = label;
        }
        for (auto& cell : group.encounters) {
            if (matches(cell.id)) {
                cell.abbreviation = label;
            }
        }
    }

    for (auto& group : fractalGroups) {
        if (matches(group.id)) {
            group.abbreviation = label;
        }
        for (auto& cell : group.encounters) {
            if (matches(cell.id)) {
                cell.abbreviation = label;
            }
        }
    }
}

void AppState::RebuildFractalGroups() {
    fractalGroups = FractalRotationService::BuildGroups(fractalMapData, settings, &fractalPersist);
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
    mentorProgress.LoadCache();
    mentorProgress.SetActiveAccount(accountName);
    DatAssetIconService::PreloadIndicators(raidData.powerDamageAssetId, raidData.condiDamageAssetId,
                                           raidData.defianceAssetId, raidData.mentorAssetId);
}

void AppState::LoadInstabilitiesData() {
    instabilitiesDataReady = InstabilitiesData::LoadOrDownload(addonDir, instabilitiesData);
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

void AppState::RequestAccountResolve() {
    if (characterName.empty()) return;
    if (accountResolveInFlight.exchange(true)) return;

    const auto character = characterName;
    std::thread([this, character]() {
        Gw2ApiClient client;
        const bool changed = accountRegistry.ResolveActiveAccountWithNetwork(character, client);
        accountResolveInFlight.store(false);
        if (changed) {
            pendingAccountChanged.store(true);
        } else {
            pendingTooltipRefresh.store(true);
        }
    }).detach();
}

void AppState::OnApiPoll() {
    const auto activeKey = accountRegistry.ActiveApiKey();
    if (!activeKey) {
        accountName.clear();
        std::lock_guard lock(dataMutex);
        raidClearsSet.clear();
        strikeClearsSet.clear();
        fractalClearsSet.clear();
        dungeonClearsSet.clear();
        frequenterPathsSet.clear();
        completedBountyAchievementIds.clear();
        ApplyRaidClears();
        ApplyStrikeClears();
        ApplyFractalClears();
        ApplyDungeonClears();
        ApplyDailyBountyClears();
        ApplyNonWeeklyHighlights();
        if (api) QuickAccessService::Refresh(api, *this);
        return;
    }

    gw2Api.SetApiKey(*activeKey);
    accountName = accountRegistry.ActiveAccountName().value_or("");

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

    const auto mentorChanges = mentorProgress.RefreshFromApi(gw2Api, settings.showMentorProgress);
    if (settings.showMentorProgress && settings.showMentorProgressPopup) {
        MentorProgressPopupService::OnProgressIncreased(*this, mentorChanges);
    }

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

std::string AppState::ApiAccountsPath() const {
    return (std::filesystem::path(addonDir) / "api_accounts.json").string();
}

void AppState::OnActiveAccountChanged() {
    accountName = accountRegistry.ActiveAccountName().value_or("");
    mentorProgress.SetActiveAccount(accountName);

    if (const auto activeKey = accountRegistry.ActiveApiKey()) {
        gw2Api.SetApiKey(*activeKey);
    }

    mapWatcher.DispatchCurrentStrikeClears();
    fractalMapWatcher.DispatchCurrentFractalClears();
    RequestApiRefresh();
    if (api) QuickAccessService::Refresh(api, *this);
}

void AppState::UpdateActiveCharacter(const std::string& newCharacterName) {
    if (newCharacterName == characterName &&
        newCharacterName == accountRegistry.LastResolvedCharacter()) {
        return;
    }

    characterName = newCharacterName;
    if (accountRegistry.ResolveActiveAccountFromCache(newCharacterName)) {
        OnActiveAccountChanged();
    } else {
        accountName = accountRegistry.ActiveAccountName().value_or("");
        mentorProgress.SetActiveAccount(accountName);
        if (api) QuickAccessService::Refresh(api, *this);
        if (!newCharacterName.empty()) {
            RequestAccountResolve();
        }
    }
}

void AppState::RegisterApiKey(const std::string& apiKey) {
    accountRegistry.RegisterKeyAsync(apiKey, ApiAccountsPath(),
                                     [this](const RegisterKeyResult& result) {
                                         if (api) {
                                             const auto level =
                                                 result.success ? LOGL_INFO : LOGL_WARNING;
                                             api->Log(level, "NexusRaidClears",
                                                      result.message.c_str());
                                         }
                                         if (result.success) {
                                             pendingCharacterResolve.store(true);
                                         }
                                     });
}

void AppState::RemoveApiKey(const std::string& tokenId) {
    if (!accountRegistry.RemoveKey(tokenId)) return;
    accountRegistry.Save(ApiAccountsPath());
    UpdateActiveCharacter(characterName);
}

}  // namespace rc
