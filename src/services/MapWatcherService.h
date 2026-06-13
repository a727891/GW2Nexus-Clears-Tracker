#pragma once

#include "data/StrikeData.h"
#include "services/ResetsWatcher.h"
#include "services/StrikePersistance.h"
#include <functional>
#include <string>
#include <vector>

namespace rc {

class MapWatcherService {
public:
    using ClearsCallback = std::function<void(const std::vector<std::string>&)>;

    void Initialize(const StrikeData* strikeData,
                    StrikePersistance* persistence,
                    ResetsWatcher* resets,
                    std::function<std::string()> accountNameProvider);

    void Update(uint32_t mapId);
    void SetClearsCallback(ClearsCallback cb);
    void DispatchCurrentStrikeClears();

private:
    const StrikeData* strikeData_ = nullptr;
    StrikePersistance* persistence_ = nullptr;
    ResetsWatcher* resets_ = nullptr;
    std::function<std::string()> accountNameProvider_;
    ClearsCallback clearsCallback_;

    bool isOnStrikeMap_ = false;
    uint32_t currentMapId_ = 0;
    std::string currentStrikeId_;
    uint32_t lastMapId_ = 0;

    void CompleteCurrentStrike();
};

}  // namespace rc
