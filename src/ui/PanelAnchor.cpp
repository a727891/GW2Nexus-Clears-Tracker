#include "ui/PanelAnchor.h"

#include <cmath>

namespace rc {
namespace PanelAnchor {

namespace {

float SnapPos(float value) { return std::roundf(value); }

}  // namespace

void AlignChildToParent(WindowState& child,
                        const WindowState& parent,
                        ImVec2 parentSize,
                        PanelLayout layout) {
    if (layout == PanelLayout::Horizontal) {
        child.posX = SnapPos(parent.posX + parentSize.x + kPanelPadding);
        child.posY = SnapPos(parent.posY);
        return;
    }

    child.posX = SnapPos(parent.posX);
    child.posY = SnapPos(parent.posY + parentSize.y + kPanelPadding);
}

void AlignStrikesToRaid(SettingsStore& settings, ImVec2 raidSize) {
    AlignChildToParent(settings.strikesPanel, settings.raidPanel, raidSize,
                       settings.strikesAppearance.panelLayout);
}

void AlignFractalsToStrikes(SettingsStore& settings, ImVec2 strikeSize) {
    AlignChildToParent(settings.fractalsPanel, settings.strikesPanel, strikeSize,
                       settings.fractalsAppearance.panelLayout);
}

void OnRaidDragged(SettingsStore& settings, ImVec2 raidSize, ImVec2 strikeSize) {
    if (settings.anchorStrikesToRaidPanel) {
        AlignStrikesToRaid(settings, raidSize);
    }
    if (settings.anchorFractalsToStrikesPanel) {
        AlignFractalsToStrikes(settings, strikeSize);
    }
}

void OnStrikesDragged(SettingsStore& settings, ImVec2 raidSize, ImVec2 strikeSize) {
    if (settings.anchorStrikesToRaidPanel) {
        if (settings.strikesAppearance.panelLayout == PanelLayout::Horizontal) {
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

    if (settings.anchorFractalsToStrikesPanel) {
        AlignFractalsToStrikes(settings, strikeSize);
    }
}

void OnFractalsDragged(SettingsStore& settings, ImVec2 raidSize, ImVec2 strikeSize) {
    if (!settings.anchorFractalsToStrikesPanel) return;

    if (settings.fractalsAppearance.panelLayout == PanelLayout::Horizontal) {
        settings.strikesPanel.posX =
            SnapPos(settings.fractalsPanel.posX - strikeSize.x - kPanelPadding);
        settings.strikesPanel.posY = SnapPos(settings.fractalsPanel.posY);
    } else {
        settings.strikesPanel.posX = SnapPos(settings.fractalsPanel.posX);
        settings.strikesPanel.posY =
            SnapPos(settings.fractalsPanel.posY - strikeSize.y - kPanelPadding);
    }

    if (settings.anchorStrikesToRaidPanel) {
        if (settings.strikesAppearance.panelLayout == PanelLayout::Horizontal) {
            settings.raidPanel.posX =
                SnapPos(settings.strikesPanel.posX - raidSize.x - kPanelPadding);
            settings.raidPanel.posY = SnapPos(settings.strikesPanel.posY);
        } else {
            settings.raidPanel.posX = SnapPos(settings.strikesPanel.posX);
            settings.raidPanel.posY =
                SnapPos(settings.strikesPanel.posY - raidSize.y - kPanelPadding);
        }
    }

    AlignFractalsToStrikes(settings, strikeSize);
}

}  // namespace PanelAnchor
}  // namespace rc
