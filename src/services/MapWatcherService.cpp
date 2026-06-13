#include "services/MapWatcherService.h"

namespace rc {

void MapWatcherService::Initialize(const StrikeData* strikeData,
                                   StrikePersistance* persistence,
                                   ResetsWatcher* resets,
                                   std::function<std::string()> accountNameProvider) {
    strikeData_ = strikeData;
    persistence_ = persistence;
    resets_ = resets;
    accountNameProvider_ = std::move(accountNameProvider);
}

void MapWatcherService::SetClearsCallback(ClearsCallback cb) { clearsCallback_ = std::move(cb); }

void MapWatcherService::Update(uint32_t mapId) {
    if (!strikeData_ || !persistence_ || !resets_) return;

    const auto* mission = strikeData_->GetMissionByMapId(mapId);
    if (mission) {
        if (isOnStrikeMap_ && !currentStrikeId_.empty() &&
            currentStrikeId_ != mission->id) {
            CompleteCurrentStrike();
        }
        isOnStrikeMap_ = true;
        currentStrikeId_ = mission->id;
        currentMapId_ = mapId;
    } else if (isOnStrikeMap_) {
        CompleteCurrentStrike();
        isOnStrikeMap_ = false;
        currentStrikeId_.clear();
        currentMapId_ = 0;
    }
    lastMapId_ = mapId;
}

void MapWatcherService::CompleteCurrentStrike() {
    if (!isOnStrikeMap_ || currentStrikeId_.empty()) return;
    if (!strikeData_->IsMapTracked(currentStrikeId_)) return;

    const auto account = accountNameProvider_ ? accountNameProvider_() : "";
    if (account.empty()) return;

    persistence_->SaveClear(account, currentStrikeId_);
    DispatchCurrentStrikeClears();
}

void MapWatcherService::DispatchCurrentStrikeClears() {
    if (!strikeData_ || !persistence_ || !resets_ || !clearsCallback_) return;

    const auto account = accountNameProvider_ ? accountNameProvider_() : "";
    if (account.empty()) return;

    const auto clears = persistence_->GetClearsForAccount(account, *strikeData_);
    std::vector<std::string> clearedIds;
    const auto lastDaily = resets_->LastDailyReset();
    const auto lastWeekly = resets_->LastWeeklyReset();

    for (const auto& [id, time] : clears) {
        if (time == std::chrono::system_clock::time_point{}) continue;
        const auto resetType = strikeData_->GetMissionResetById(id);
        if (resetType == "daily") {
            if (time >= lastDaily) clearedIds.push_back(id);
        } else if (time >= lastWeekly) {
            clearedIds.push_back(id);
        }
    }
    clearsCallback_(clearedIds);
}

}  // namespace rc
