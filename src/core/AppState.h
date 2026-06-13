#pragma once

#include "core/SettingsStore.h"
#include "core/RaidVisibilityStore.h"
#include "core/StrikeVisibilityStore.h"
#include "data/DailyBountyData.h"
#include "data/RaidData.h"
#include "data/StrikeData.h"
#include "services/ApiPollService.h"
#include "services/DailyBountyProgressService.h"
#include "services/Gw2ApiClient.h"
#include "services/MapWatcherService.h"
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

    SettingsStore settings;
    RaidVisibilityStore raidVisibility;
    StrikeVisibilityStore strikeVisibility;
    RaidData raidData;
    StrikeData strikeData;
    DailyBountyData dailyBountyData;

    Gw2ApiClient gw2Api;
    ApiPollService apiPoll;
    ResetsWatcher resets;
    std::chrono::system_clock::time_point trackedDailyReset_;
    std::chrono::system_clock::time_point trackedWeeklyReset_;
    StrikePersistance strikePersist;
    DailyBountyProgressService dailyBountyProgress;
    WeeklyBountyEncountersService weeklyBountyEncounters;
    MapWatcherService mapWatcher;

    std::vector<GridGroup> raidGroups;
    std::vector<GridGroup> strikeGroups;

    std::mutex dataMutex;
    std::unordered_set<std::string> raidClearsSet;
    std::unordered_set<std::string> strikeClearsSet;
    std::unordered_set<int> completedBountyAchievementIds;
    std::atomic<bool> apiRefreshPending{false};
    std::atomic<bool> staticDataLoadPending{false};
    bool staticDataReady = false;

    void Initialize(AddonAPI_t* apiPtr);
    void Shutdown();
    bool LoadStaticDataFromCache();
    void LoadStaticDataWithNetwork();
    void RequestStaticDataLoad();
    void ProcessPendingStaticDataLoad();
    void RebuildRaidGroups();
    void RebuildStrikeGroups();
    void SyncEncounterVisibility();
    void OnApiPoll();
    void ApplyRaidClears();
    void ApplyNonWeeklyHighlights();
    void ApplyStrikeClears();
    void ApplyDailyBountyClears();
    bool ShouldShowPanel(bool panelVisible) const;
    void RequestApiRefresh();
    void ProcessPendingApiRefresh();
    void TickResets();
};

}  // namespace rc
