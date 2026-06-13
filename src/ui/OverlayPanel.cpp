#include "ui/OverlayPanel.h"

#include <cmath>

namespace rc {
namespace OverlayPanel {

namespace {

constexpr ImGuiWindowFlags kOverlayFlags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
    ImGuiWindowFlags_NoSavedSettings;

WindowState* g_windowState = nullptr;
PanelRole g_activeRole = PanelRole::Raid;
int g_styleVarCount = 0;
bool g_raidDragging = false;
bool g_strikesDragging = false;

struct DragState {
    bool active = false;
    ImVec2 grabOffset{};
};

DragState g_raidDrag;
DragState g_strikesDrag;

float SnapPos(float value) { return std::roundf(value); }

ImVec2 SnapPos(const ImVec2& value) {
    return {SnapPos(value.x), SnapPos(value.y)};
}

ImVec2 Subtract(const ImVec2& a, const ImVec2& b) {
    return {a.x - b.x, a.y - b.y};
}

DragState& DragFor(PanelRole role) {
    return role == PanelRole::Raid ? g_raidDrag : g_strikesDrag;
}

}  // namespace

bool Begin(const char* id, WindowState& state, ImVec2 contentSize, PanelRole role) {
    (void)contentSize;
    g_windowState = &state;
    g_activeRole = role;

    ImGui::SetNextWindowPos(SnapPos(ImVec2(state.posX, state.posY)), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    g_styleVarCount = 2;

    if (!ImGui::Begin(id, nullptr, kOverlayFlags)) {
        ImGui::End();
        ImGui::PopStyleVar(g_styleVarCount);
        g_styleVarCount = 0;
        g_windowState = nullptr;
        return false;
    }

    return true;
}

bool End(PanelRole role) {
    bool dragged = false;
    if (g_windowState) {
        DragState& drag = DragFor(role);
        const bool hovered =
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            drag.active = true;
            drag.grabOffset = Subtract(ImGui::GetMousePos(), ImGui::GetWindowPos());
        }

        if (drag.active && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            const ImVec2 newPos = SnapPos(Subtract(ImGui::GetMousePos(), drag.grabOffset));
            ImGui::SetWindowPos(newPos);
            dragged = true;
        } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            drag.active = false;
        }

        const ImVec2 pos = SnapPos(ImGui::GetWindowPos());
        g_windowState->posX = pos.x;
        g_windowState->posY = pos.y;
    }

    if (role == PanelRole::Raid) {
        g_raidDragging = dragged;
    } else {
        g_strikesDragging = dragged;
    }

    ImGui::End();
    if (g_styleVarCount > 0) {
        ImGui::PopStyleVar(g_styleVarCount);
        g_styleVarCount = 0;
    }
    g_windowState = nullptr;
    return dragged;
}

bool IsDragging(PanelRole role) {
    return role == PanelRole::Raid ? g_raidDragging : g_strikesDragging;
}

}  // namespace OverlayPanel
}  // namespace rc
