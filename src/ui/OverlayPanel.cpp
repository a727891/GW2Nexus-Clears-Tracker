#include "ui/OverlayPanel.h"

#include <cmath>

namespace rc {
namespace OverlayPanel {

namespace {

constexpr ImGuiWindowFlags kOverlayBaseFlags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
    ImGuiWindowFlags_NoSavedSettings;

WindowState* g_windowState = nullptr;
PanelRole g_activeRole = PanelRole::Raid;
bool g_lockPosition = false;
int g_styleVarCount = 0;
bool g_dragging[4] = {};

struct DragState {
    bool active = false;
    ImVec2 grabOffset{};
};

DragState g_dragStates[4];

float SnapPos(float value) { return std::roundf(value); }

ImVec2 SnapPos(const ImVec2& value) {
    return {SnapPos(value.x), SnapPos(value.y)};
}

ImVec2 Subtract(const ImVec2& a, const ImVec2& b) {
    return {a.x - b.x, a.y - b.y};
}

void ClampPosition(WindowState& state, const ImVec2& windowSize, uint32_t screenWidth,
                   uint32_t screenHeight) {
    if (screenWidth == 0 || screenHeight == 0) return;

    const float maxX = static_cast<float>(screenWidth) - windowSize.x;
    const float maxY = static_cast<float>(screenHeight) - windowSize.y;
    if (state.posX < 0.0f) state.posX = 0.0f;
    if (state.posY < 0.0f) state.posY = 0.0f;
    if (state.posX > maxX) state.posX = maxX;
    if (state.posY > maxY) state.posY = maxY;
}

int RoleIndex(PanelRole role) { return static_cast<int>(role); }

DragState& DragFor(PanelRole role) { return g_dragStates[RoleIndex(role)]; }

}  // namespace

bool Begin(const char* id, WindowState& state, ImVec2 contentSize, PanelRole role,
           bool lockPosition) {
    (void)contentSize;
    g_windowState = &state;
    g_activeRole = role;
    g_lockPosition = lockPosition;

    ImGui::SetNextWindowPos(SnapPos(ImVec2(state.posX, state.posY)), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    g_styleVarCount = 2;

    ImGuiWindowFlags flags = kOverlayBaseFlags;
    if (lockPosition) {
        flags |= ImGuiWindowFlags_NoInputs;
    }

    if (!ImGui::Begin(id, nullptr, flags)) {
        ImGui::End();
        ImGui::PopStyleVar(g_styleVarCount);
        g_styleVarCount = 0;
        g_windowState = nullptr;
        return false;
    }

    return true;
}

bool End(PanelRole role, bool screenClamp, uint32_t screenWidth, uint32_t screenHeight) {
    bool dragged = false;
    if (g_windowState) {
        DragState& drag = DragFor(role);

        if (g_lockPosition) {
            drag.active = false;
        } else {
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
        }

        const ImVec2 pos = SnapPos(ImGui::GetWindowPos());
        const ImVec2 size = ImGui::GetWindowSize();
        g_windowState->posX = pos.x;
        g_windowState->posY = pos.y;
        if (screenClamp) {
            ClampPosition(*g_windowState, size, screenWidth, screenHeight);
            if (g_windowState->posX != pos.x || g_windowState->posY != pos.y) {
                ImGui::SetWindowPos(SnapPos(ImVec2(g_windowState->posX, g_windowState->posY)));
            }
        }
    }

    g_dragging[RoleIndex(role)] = dragged;

    ImGui::End();
    if (g_styleVarCount > 0) {
        ImGui::PopStyleVar(g_styleVarCount);
        g_styleVarCount = 0;
    }
    g_windowState = nullptr;
    return dragged;
}

bool IsDragging(PanelRole role) { return g_dragging[RoleIndex(role)]; }

}  // namespace OverlayPanel
}  // namespace rc
