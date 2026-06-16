#pragma once

#include "nexus/Nexus.h"

namespace rc {

class AppState;

namespace QuickAccessService {
    void Register(AddonAPI_t* api, AppState& state);
    void Unregister(AddonAPI_t* api);
    void SyncVisibility(AddonAPI_t* api, AppState& state);
    void Refresh(AddonAPI_t* api, AppState& state);
    void OnShortcutActivated(AppState& state);
    bool IsRegistered();
}

}  // namespace rc
