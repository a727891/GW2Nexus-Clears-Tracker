#include "ui/EncounterTooltip.h"

#include "services/DatAssetIconService.h"

namespace rc {
namespace EncounterTooltip {

namespace {

constexpr float kTooltipWidth = 230.0f;
constexpr float kBossIconSize = 48.0f;
constexpr float kRowIconSize = 20.0f;
constexpr float kPadding = 5.0f;
constexpr float kRowHeight = 25.0f;
constexpr float kMentorGap = 12.0f;

void DrawIcon(ImDrawList* draw, ImTextureID texture, const ImVec2& pos, float size) {
    if (!texture) return;
    draw->AddImage(texture, pos, ImVec2(pos.x + size, pos.y + size));
}

void DrawIndicatorRow(ImDrawList* draw,
                      float x,
                      float& y,
                      ImTextureID icon,
                      const char* label,
                      ImU32 textColor) {
    DrawIcon(draw, icon, ImVec2(x, y), kRowIconSize);
    draw->AddText(ImVec2(x + kRowIconSize + 5.0f, y + 2.0f), textColor, label);
    y += kRowHeight;
}

int TooltipHeight(const EncounterTooltipData& data,
                    const RaidData& raidData,
                    bool showMentorProgress) {
    int height = 58;
    int indicatorCount = 0;
    if (data.powerFavored) ++indicatorCount;
    if (data.condiFavored) ++indicatorCount;
    if (data.needsDefianceBreak && raidData.defianceAssetId > 0) ++indicatorCount;
    height += indicatorCount * static_cast<int>(kRowHeight);

    const bool hasOtherCallouts = indicatorCount > 0;
    if (showMentorProgress && !data.isStrike && data.mentorAchievementId) {
        if (hasOtherCallouts) height += static_cast<int>(kMentorGap);
        height += static_cast<int>(kRowHeight);
    }
    return height;
}

}  // namespace

std::optional<EncounterTooltipData> BuildFromRaid(const RaidData& raidData,
                                                  const std::string& encounterId) {
    const BossEncounter* enc = raidData.GetEncounterById(encounterId);
    if (!enc) return std::nullopt;

    EncounterTooltipData data;
    data.name = enc->name;
    data.abbreviation = enc->abbreviation;
    data.assetId = enc->assetId;
    data.powerFavored = enc->powerFavored;
    data.condiFavored = enc->condiFavored;
    data.needsDefianceBreak = enc->needsDefianceBreak;
    data.mentorAchievementId = enc->mentorAchievementId;
    data.mentorAchievementMax = enc->mentorAchievementMax;
    data.isStrike = false;
    return data;
}

std::optional<EncounterTooltipData> BuildFromStrike(const StrikeData& strikeData,
                                                     const std::string& encounterId) {
    const StrikeMission* mission = strikeData.GetMissionById(encounterId);
    if (!mission) return std::nullopt;

    EncounterTooltipData data;
    data.name = mission->name;
    data.abbreviation = mission->abbreviation;
    data.assetId = mission->assetId;
    data.powerFavored = mission->powerFavored;
    data.condiFavored = mission->condiFavored;
    data.needsDefianceBreak = mission->needsDefianceBreak;
    data.isStrike = true;
    return data;
}

void ShowIfHovered(const ImVec2& p0,
                   const ImVec2& p1,
                   const EncounterTooltipData& data,
                   const RaidData& raidData,
                   const MentorAchievementProgressService* mentorProgress,
                   bool showMentorProgress) {
    if (!ImGui::IsMouseHoveringRect(p0, p1)) return;

    const int height = TooltipHeight(data, raidData, showMentorProgress);
    ImGui::SetNextWindowSize(ImVec2(kTooltipWidth, static_cast<float>(height)));
    ImGui::BeginTooltip();

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    const ImTextureID bossIcon = DatAssetIconService::Request(data.assetId);
    DrawIcon(draw, bossIcon, origin, kBossIconSize);

    const float textX = origin.x + kBossIconSize + kPadding;
    const ImU32 titleColor = IM_COL32(255, 212, 163, 255);
    const ImU32 subtitleColor = IM_COL32(255, 255, 255, 204);
    const ImU32 rowColor = IM_COL32(255, 255, 255, 230);
    const ImU32 defianceColor = IM_COL32(57, 172, 161, 255);

    draw->AddText(ImVec2(textX, origin.y + kPadding), titleColor, data.name.c_str());

    const std::string subtitle = "(" + data.abbreviation + ")";
    draw->AddText(ImVec2(textX, origin.y + kPadding + ImGui::GetTextLineHeight()),
                  subtitleColor, subtitle.c_str());

    float rowY = origin.y + kBossIconSize + kPadding;
    const float rowX = origin.x;

    if (data.powerFavored) {
        const ImTextureID icon = DatAssetIconService::Request(raidData.powerDamageAssetId);
        DrawIndicatorRow(draw, rowX, rowY, icon, "Power Damage", rowColor);
    }
    if (data.condiFavored) {
        const ImTextureID icon = DatAssetIconService::Request(raidData.condiDamageAssetId);
        DrawIndicatorRow(draw, rowX, rowY, icon, "Condition Damage", rowColor);
    }
    if (data.needsDefianceBreak && raidData.defianceAssetId > 0) {
        const ImTextureID icon = DatAssetIconService::Request(raidData.defianceAssetId);
        DrawIndicatorRow(draw, rowX, rowY, icon, "Defiance Break", defianceColor);
    }

    const bool hasOtherCallouts =
        data.powerFavored || data.condiFavored ||
        (data.needsDefianceBreak && raidData.defianceAssetId > 0);

    if (showMentorProgress && !data.isStrike && data.mentorAchievementId) {
        if (hasOtherCallouts) rowY += kMentorGap;

        const int mentorId = *data.mentorAchievementId;
        ImTextureID mentorIcon = nullptr;
        if (raidData.mentorAssetId > 0) {
            mentorIcon = DatAssetIconService::Request(raidData.mentorAssetId);
        }

        char mentorText[64];
        if (mentorProgress) {
            const auto progress = mentorProgress->GetProgress(mentorId);
            if (progress && progress->done) {
                snprintf(mentorText, sizeof(mentorText), "Mentor: Done");
            } else if (progress) {
                snprintf(mentorText, sizeof(mentorText), "Mentor: %d / %d", progress->current,
                         progress->max);
            } else {
                const int max = data.mentorAchievementMax.value_or(
                    MentorAchievementProgressService::kDefaultMax);
                snprintf(mentorText, sizeof(mentorText), "Mentor: 0 / %d", max);
            }
        } else {
            const int max =
                data.mentorAchievementMax.value_or(MentorAchievementProgressService::kDefaultMax);
            snprintf(mentorText, sizeof(mentorText), "Mentor: 0 / %d", max);
        }

        if (mentorIcon) {
            DrawIndicatorRow(draw, rowX, rowY, mentorIcon, mentorText, rowColor);
        } else {
            draw->AddText(ImVec2(rowX + 5.0f, rowY + 2.0f), rowColor, mentorText);
            rowY += kRowHeight;
        }
    }

    ImGui::Dummy(ImVec2(kTooltipWidth - kPadding * 2.0f, static_cast<float>(height) - kPadding));
    ImGui::EndTooltip();
}

}  // namespace EncounterTooltip
}  // namespace rc
