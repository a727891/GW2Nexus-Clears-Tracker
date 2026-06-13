#pragma once

#include "core/SettingsStore.h"

#include <imgui.h>

namespace rc {
namespace PanelAnchor {

constexpr float kPanelPadding = 2.0f;

void AlignChildToParent(WindowState& child,
                        const WindowState& parent,
                        ImVec2 parentSize,
                        PanelLayout layout);
void AlignStrikesToRaid(SettingsStore& settings, ImVec2 raidSize);
void AlignFractalsToStrikes(SettingsStore& settings, ImVec2 strikeSize);
void OnRaidDragged(SettingsStore& settings, ImVec2 raidSize, ImVec2 strikeSize);
void OnStrikesDragged(SettingsStore& settings, ImVec2 raidSize, ImVec2 strikeSize);
void OnFractalsDragged(SettingsStore& settings, ImVec2 raidSize, ImVec2 strikeSize);

}  // namespace PanelAnchor
}  // namespace rc
