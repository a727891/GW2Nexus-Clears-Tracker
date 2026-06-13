#pragma once

#include "data/FractalMapData.h"
#include "services/FractalPersistance.h"
#include "services/ResetsWatcher.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace rc {

class FractalMapWatcherService {
public:
    using ClearsCallback = std::function<void(const std::vector<std::string>&)>;

    void Initialize(const FractalMapData* fractalData,
                    FractalPersistance* persistence,
                    ResetsWatcher* resets,
                    std::function<std::string()> accountNameProvider);
    void SetClearsCallback(ClearsCallback cb);
    void Update(uint32_t mapId);
    void DispatchCurrentFractalClears();

private:
    void CompleteCurrentFractal();

    const FractalMapData* fractalData_ = nullptr;
    FractalPersistance* persistence_ = nullptr;
    ResetsWatcher* resets_ = nullptr;
    std::function<std::string()> accountNameProvider_;
    ClearsCallback clearsCallback_;

    bool isOnFractalMap_ = false;
    std::string currentApiLabel_;
    uint32_t currentMapId_ = 0;
};

}  // namespace rc
