#include "ui/PanelAnchor.h"

#include <cmath>

namespace rc {
namespace PanelAnchor {

namespace {

float SnapPos(float value) { return std::roundf(value); }

}  // namespace

void AlignStrikesToRaid(SettingsStore& settings, ImVec2 raidSize) {
    if (settings.panelLayout == PanelLayout::Horizontal) {
        settings.strikesPanel.posX =
            SnapPos(settings.raidPanel.posX + raidSize.x + kPanelPadding);
        settings.strikesPanel.posY = SnapPos(settings.raidPanel.posY);
        return;
    }

    settings.strikesPanel.posX = SnapPos(settings.raidPanel.posX);
    settings.strikesPanel.posY =
        SnapPos(settings.raidPanel.posY + raidSize.y + kPanelPadding);
}

void OnRaidDragged(SettingsStore& settings, ImVec2 raidSize) {
    if (!settings.anchorStrikesToRaidPanel) return;
    AlignStrikesToRaid(settings, raidSize);
}

void OnStrikesDragged(SettingsStore& settings, ImVec2 raidSize) {
    if (!settings.anchorStrikesToRaidPanel) return;

    if (settings.panelLayout == PanelLayout::Horizontal) {
        settings.raidPanel.posX =
            SnapPos(settings.strikesPanel.posX - raidSize.x - kPanelPadding);
        settings.raidPanel.posY = SnapPos(settings.strikesPanel.posY);
    } else {
        settings.raidPanel.posX = SnapPos(settings.strikesPanel.posX);
        settings.raidPanel.posY =
            SnapPos(settings.strikesPanel.posY - raidSize.y - kPanelPadding);
    }

    AlignStrikesToRaid(settings, raidSize);
}

}  // namespace PanelAnchor
}  // namespace rc
