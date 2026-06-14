#include "ui/MentorProgressPopupService.h"

#include "services/DatAssetIconService.h"

#include <filesystem>
#include <imgui.h>

namespace rc {
namespace MentorProgressPopupService {
namespace {

constexpr float kPopupWidth = 300.0f;
constexpr float kPopupHeight = 72.0f;
constexpr float kPopupGap = 8.0f;
constexpr float kPopupMargin = 20.0f;
constexpr float kAutoCloseSeconds = 5.0f;
constexpr float kIconSize = 48.0f;
constexpr float kPadding = 8.0f;

struct ActivePopup {
    std::string bossName;
    int current = 0;
    int max = 0;
    int delta = 0;
    int iconAssetId = 0;
    float openedAt = 0.0f;
    ImVec2 pos{};
};

struct ExamplePopupState {
    ImVec2 pos{};
    bool initialized = false;
    bool dragging = false;
    ImVec2 grabOffset{};
};

std::vector<ActivePopup> g_popups;
ExamplePopupState g_example;

ImVec2 DefaultPopupPosition(uint32_t screenWidth) {
    const float x = screenWidth > 0 ? static_cast<float>(screenWidth) - kPopupWidth - kPopupMargin
                                    : 100.0f;
    return {x, kPopupMargin + 60.0f};
}

ImVec2 SavedOrDefaultPosition(const SettingsStore& settings, uint32_t screenWidth) {
    if (settings.mentorProgressPopupPosX >= 0.0f && settings.mentorProgressPopupPosY >= 0.0f) {
        return {settings.mentorProgressPopupPosX, settings.mentorProgressPopupPosY};
    }
    return DefaultPopupPosition(screenWidth);
}

void DrawPopupContent(const ActivePopup& popup) {
    ImTextureID icon = ImTextureID{};
    if (popup.iconAssetId > 0) {
        icon = DatAssetIconService::Request(popup.iconAssetId);
    }

    const float contentLeft = kPadding + kIconSize + kPadding;
    if (icon) {
        ImGui::SetCursorPos({kPadding, (kPopupHeight - kIconSize) * 0.5f});
        ImGui::Image(icon, {kIconSize, kIconSize});
    }

    ImGui::SetCursorPos({contentLeft, kPadding});
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(250, 250, 210, 255));
    ImGui::TextUnformatted(popup.bossName.c_str());
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({contentLeft, kPadding + 20.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(218, 165, 32, 255));
    ImGui::Text("+%d", popup.delta);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(250, 250, 210, 255));
    ImGui::Text("%d / %d", popup.current, popup.max);
    ImGui::PopStyleColor();
}

bool RenderPopupWindow(const char* id, ImVec2& pos, bool draggable, bool* open,
                       const ActivePopup* content) {
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize({kPopupWidth, kPopupHeight}, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoSavedSettings;
    if (!draggable) {
        flags |= ImGuiWindowFlags_NoMove;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(20, 20, 20, 220));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    const bool visible = ImGui::Begin(id, open, flags);
    if (visible) {
        pos = ImGui::GetWindowPos();
        if (draggable) {
            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                g_example.dragging = true;
                g_example.grabOffset = {ImGui::GetMousePos().x - pos.x,
                                        ImGui::GetMousePos().y - pos.y};
            }
            if (g_example.dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                pos = {ImGui::GetMousePos().x - g_example.grabOffset.x,
                       ImGui::GetMousePos().y - g_example.grabOffset.y};
                ImGui::SetWindowPos(pos);
            } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                g_example.dragging = false;
            }
        } else if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
                   ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            *open = false;
        }

        if (content) {
            DrawPopupContent(*content);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    return visible;
}

}  // namespace

void OnProgressIncreased(AppState& state, const std::vector<MentorProgressChange>& changes) {
    if (!state.settings.showMentorProgress || !state.settings.showMentorProgressPopup ||
        state.settings.mentorProgressPopupReposition) {
        return;
    }

    const uint32_t screenWidth = state.nexusLink ? state.nexusLink->Width : 0;
    ImVec2 pos = SavedOrDefaultPosition(state.settings, screenWidth);
    const float now = static_cast<float>(ImGui::GetTime());

    for (const auto& change : changes) {
        if (change.Delta() <= 0) continue;

        const BossEncounter* encounter =
            state.raidData.GetEncounterByMentorAchievementId(change.achievementId);
        const std::string bossName =
            encounter ? encounter->name : "Achievement " + std::to_string(change.achievementId);
        const int max = state.mentorProgress.GetMaxForAchievement(change.achievementId);
        const int iconAssetId =
            encounter && encounter->assetId > 0 ? encounter->assetId : state.raidData.mentorAssetId;

        g_popups.push_back({bossName, change.newCurrent, max, change.Delta(), iconAssetId, now,
                            pos});
        pos.y += kPopupHeight + kPopupGap;
    }
}

void SaveExamplePosition(AppState& state) {
    if (g_example.initialized) {
        state.settings.mentorProgressPopupPosX = g_example.pos.x;
        state.settings.mentorProgressPopupPosY = g_example.pos.y;
        const auto settingsPath =
            (std::filesystem::path(state.addonDir) / "settings.json").string();
        state.settings.Save(settingsPath);
    }
}

void Render(AppState& state) {
    const uint32_t screenWidth = state.nexusLink ? state.nexusLink->Width : 0;
    const float now = static_cast<float>(ImGui::GetTime());

    if (state.settings.mentorProgressPopupReposition) {
        if (!g_example.initialized) {
            g_example.pos = SavedOrDefaultPosition(state.settings, screenWidth);
            g_example.initialized = true;
        }

        bool open = true;
        ActivePopup example{"Example Boss", 99, 1000, 1, 1203237, now, g_example.pos};
        RenderPopupWindow("MentorProgressExample", g_example.pos, true, &open, &example);
        return;
    }

    g_example.initialized = false;
    g_example.dragging = false;

    for (size_t i = 0; i < g_popups.size();) {
        ActivePopup& popup = g_popups[i];
        if (now - popup.openedAt >= kAutoCloseSeconds) {
            g_popups.erase(g_popups.begin() + static_cast<std::ptrdiff_t>(i));
            continue;
        }

        bool open = true;
        RenderPopupWindow(("MentorProgress##" + std::to_string(i)).c_str(), popup.pos, false, &open,
                          &popup);

        if (!open) {
            g_popups.erase(g_popups.begin() + static_cast<std::ptrdiff_t>(i));
            continue;
        }

        ++i;
    }
}

}  // namespace MentorProgressPopupService
}  // namespace rc
