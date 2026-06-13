#pragma once

#include "data/RaidData.h"

namespace rc {

struct WeeklyModifier {
    bool emboldened = false;
    bool callOfTheMists = false;
};

namespace WeeklyModifierService {

bool IsModifierActive(int timestamp, int weeksBetween, int secondsInWeek);
WeeklyModifier ForWing(const RaidWing& wing, int secondsInWeek);

}  // namespace WeeklyModifierService
}  // namespace rc
