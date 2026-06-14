#pragma once

#include "core/AccountRegistry.h"
#include "core/SettingsStore.h"
#include "core/RaidVisibilityStore.h"
#include "core/StrikeVisibilityStore.h"
#include "data/DailyBountyData.h"
#include "data/DungeonData.h"
#include "data/FractalMapData.h"
#include "data/RaidData.h"
#include "data/StrikeData.h"
#include "services/ApiPollService.h"
#include "services/DailyBountyProgressService.h"
#include "services/DatAssetIconService.h"
#include "services/FractalMapWatcherService.h"
#include "services/FractalPersistance.h"
#include "services/Gw2ApiClient.h"
#include "services/MapWatcherService.h"
#include "services/MentorAchievementProgressService.h"
#include "services/RaidClearsService.h"
#include "services/ResetsWatcher.h"
#include "services/StrikeClearsService.h"
#include "services/StrikePersistance.h"
#include "services/WeeklyBountyEncountersService.h"

#include "mumble/Mumble.h"
#include "nexus/Nexus.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace rc {

class AppState {
public:
    static AppState& Instance();

    AddonAPI_t* api = nullptr;
    NexusLinkData_t* nexusLink = nullptr;
    Mumble::Data* mumbleLink = nullptr;
    std::string addonDir;
    std::string accountName;
    std::string characterName;
    std::string motd;
    std::string motdId;

    SettingsStore settings;
    AccountRegistry accountRegistry;
    RaidVisibilityStore raidVisibility;
    StrikeVisibilityStore strikeVisibility;
    RaidData raidData;
    StrikeData strikeData;
    DailyBountyData dailyBountyData;
    FractalMapData fractalMapData;

    Gw2ApiClient gw2Api;
    ApiPollService apiPoll;
    ResetsWatcher resets;
    std::chrono::system_clock::time_point trackedDailyReset_;
    std::chrono::system_clock::time_point trackedWeeklyReset_;
    StrikePersistance strikePersist;
    FractalPersistance fractalPersist;
    DailyBountyProgressService dailyBountyProgress;
    WeeklyBountyEncountersService weeklyBountyEncounters;
    MapWatcherService mapWatcher;
    FractalMapWatcherService fractalMapWatcher;
    MentorAchievementProgressService mentorProgress;

    std::vector<GridGroup> raidGroups;
    std::vector<GridGroup> strikeGroups;
    std::vector<GridGroup> fractalGroups;
    std::vector<GridGroup> dungeonGroups;

    std::mutex dataMutex;
    std::unordered_set<std::string> raidClearsSet;
    std::unordered_set<std::string> strikeClearsSet;
    std::unordered_set<std::string> fractalClearsSet;
    std::unordered_set<std::string> dungeonClearsSet;
    std::unordered_set<std::string> frequenterPathsSet;
    std::unordered_set<int> completedBountyAchievementIds;
    std::atomic<bool> apiRefreshPending{false};
    std::atomic<bool> pendingAccountRefresh{false};
    std::atomic<bool> staticDataLoadPending{false};
    bool staticDataReady = false;
    bool fractalDataReady = false;

    void Initialize(AddonAPI_t* apiPtr);
    void Shutdown();
    bool LoadStaticDataFromCache();
    void LoadStaticDataWithNetwork();
    bool LoadClearsTrackerMetadata();
    void RequestStaticDataLoad();
    void ProcessPendingStaticDataLoad();
    void RebuildRaidGroups();
    void RebuildStrikeGroups();
    void RebuildFractalGroups();
    void RebuildDungeonGroups();
    void SyncEncounterVisibility();
    void ApplyEncounterLabel(const std::string& encounterId, const std::string& label);
    void RefreshTooltipServices();
    void OnApiPoll();
    void ApplyRaidClears();
    void ApplyNonWeeklyHighlights();
    void ApplyStrikeClears();
    void ApplyFractalClears();
    void ApplyDungeonClears();
    void ApplyDailyBountyClears();
    bool ShouldShowPanel(bool panelVisible) const;
    void RequestApiRefresh();
    void ProcessPendingApiRefresh();
    void TickResets();
    void UpdateActiveCharacter(const std::string& characterName);
    void OnActiveAccountChanged();
    void RegisterApiKey(const std::string& apiKey);
    void RemoveApiKey(const std::string& tokenId);
    std::string ApiAccountsPath() const;
};

}  // namespace rc
