#include "services/FractalMapWatcherService.h"

namespace rc {

void FractalMapWatcherService::Initialize(const FractalMapData* fractalData,
                                          FractalPersistance* persistence,
                                          ResetsWatcher* resets,
                                          std::function<std::string()> accountNameProvider) {
    fractalData_ = fractalData;
    persistence_ = persistence;
    resets_ = resets;
    accountNameProvider_ = std::move(accountNameProvider);
}

void FractalMapWatcherService::SetClearsCallback(ClearsCallback cb) {
    clearsCallback_ = std::move(cb);
}

void FractalMapWatcherService::Update(uint32_t mapId) {
    if (!fractalData_ || !persistence_ || !resets_) return;

    const FractalMap* mission = fractalData_->GetFractalMapById(static_cast<int>(mapId));
    if (mission) {
        if (isOnFractalMap_ && !currentApiLabel_.empty() &&
            currentApiLabel_ != mission->apiLabel) {
            CompleteCurrentFractal();
        }
        isOnFractalMap_ = true;
        currentApiLabel_ = mission->apiLabel;
        currentMapId_ = mapId;
    } else if (isOnFractalMap_) {
        CompleteCurrentFractal();
        isOnFractalMap_ = false;
        currentApiLabel_.clear();
        currentMapId_ = 0;
    }
}

void FractalMapWatcherService::CompleteCurrentFractal() {
    if (!isOnFractalMap_ || currentApiLabel_.empty()) return;

    const auto account = accountNameProvider_ ? accountNameProvider_() : "";
    if (account.empty()) return;

    persistence_->SaveClear(account, currentApiLabel_);
    DispatchCurrentFractalClears();
}

void FractalMapWatcherService::DispatchCurrentFractalClears() {
    if (!fractalData_ || !persistence_ || !resets_ || !clearsCallback_) return;

    const auto account = accountNameProvider_ ? accountNameProvider_() : "";
    if (account.empty()) return;

    const auto clears = persistence_->GetClearsForAccount(account, *fractalData_);
    std::vector<std::string> clearedIds;
    const auto lastDaily = resets_->LastDailyReset();

    for (const auto& [id, time] : clears) {
        if (time == std::chrono::system_clock::time_point{}) continue;
        if (time >= lastDaily) {
            clearedIds.push_back(id);
        }
    }
    clearsCallback_(clearedIds);
}

}  // namespace rc
