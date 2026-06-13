#include "ui/UiFontService.h"

#include <imgui.h>

namespace rc {
namespace UiFontService {

namespace {

ImFont* g_menomoniaFont = nullptr;

void OnMenomoniaFont(const char* /*identifier*/, void* font) {
    g_menomoniaFont = static_cast<ImFont*>(font);
}

}  // namespace

void Initialize(AddonAPI_t* api, NexusLinkData_t* nexusLink) {
    (void)nexusLink;
    g_menomoniaFont = nullptr;
    if (api && api->Fonts_Get) {
        api->Fonts_Get(kMenomoniaIdentifier, OnMenomoniaFont);
    }
}

void Shutdown(AddonAPI_t* api) {
    if (api && api->Fonts_Release) {
        api->Fonts_Release(kMenomoniaIdentifier, OnMenomoniaFont);
    }
    g_menomoniaFont = nullptr;
}

ImFont* GetGridFont(NexusLinkData_t* nexusLink) {
    if (g_menomoniaFont) return g_menomoniaFont;
    if (nexusLink && nexusLink->Font) return static_cast<ImFont*>(nexusLink->Font);
    return nullptr;
}

}  // namespace UiFontService
}  // namespace rc
