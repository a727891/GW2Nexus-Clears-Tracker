#pragma once

#include "core/Types.h"

#include <imgui.h>

namespace rc {
namespace OverlayPanel {

enum class PanelRole {
    Raid,
    Strikes,
};

bool Begin(const char* id, WindowState& state, ImVec2 contentSize, PanelRole role);
bool End(PanelRole role);
bool IsDragging(PanelRole role);

}  // namespace OverlayPanel
}  // namespace rc
