#pragma once

#include "core/AppState.h"
#include "services/MentorAchievementProgressService.h"

#include <vector>

namespace rc {
namespace MentorProgressPopupService {

void OnProgressIncreased(AppState& state, const std::vector<MentorProgressChange>& changes);
void SaveExamplePosition(AppState& state);
void Render(AppState& state);

}  // namespace MentorProgressPopupService
}  // namespace rc
