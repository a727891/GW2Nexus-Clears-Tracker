#pragma once

#include "core/SettingsStore.h"
#include "core/Types.h"

namespace rc {
namespace OptionsPanelAppearance {

bool RenderFields(PanelAppearance& appearance,
                  const SettingsStore& settings,
                  bool showPreview);

}  // namespace OptionsPanelAppearance
}  // namespace rc
