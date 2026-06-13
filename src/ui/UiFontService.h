#pragma once

#include "nexus/Nexus.h"

struct ImFont;

namespace rc {
namespace UiFontService {

constexpr const char* kMenomoniaIdentifier = "MENOMONIA_S";

void Initialize(AddonAPI_t* api, NexusLinkData_t* nexusLink);
void Shutdown(AddonAPI_t* api);
ImFont* GetGridFont(NexusLinkData_t* nexusLink);

}  // namespace UiFontService
}  // namespace rc
