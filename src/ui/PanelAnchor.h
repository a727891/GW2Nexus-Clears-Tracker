#pragma once

#include "core/SettingsStore.h"

#include <imgui.h>

namespace rc {
namespace PanelAnchor {

constexpr float kPanelPadding = 2.0f;

void AlignStrikesToRaid(SettingsStore& settings, ImVec2 raidSize);
void OnRaidDragged(SettingsStore& settings, ImVec2 raidSize);
void OnStrikesDragged(SettingsStore& settings, ImVec2 raidSize);

}  // namespace PanelAnchor
}  // namespace rc
