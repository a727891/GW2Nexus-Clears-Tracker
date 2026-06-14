#pragma once

#include "core/SettingsStore.h"
#include "core/Types.h"
#include "data/FractalMapData.h"

namespace rc {

class FractalPersistance;

class FractalRotationService {
public:
    static std::vector<GridGroup> BuildGroups(const FractalMapData& data,
                                              const SettingsStore& settings,
                                              const FractalPersistance* labels = nullptr);
};

}  // namespace rc
