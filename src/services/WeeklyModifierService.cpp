#include "services/WeeklyModifierService.h"

#include <chrono>
#include <cmath>

namespace rc {
namespace WeeklyModifierService {

namespace {

int64_t CurrentUnixTime() {
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

}  // namespace

bool IsModifierActive(int timestamp, int weeksBetween, int secondsInWeek) {
    if (timestamp <= 0 || weeksBetween <= 0 || secondsInWeek <= 0) return false;

    const int64_t duration = CurrentUnixTime() - static_cast<int64_t>(timestamp);
    if (duration < 0) return false;

    const int wing = static_cast<int>(std::floor(static_cast<double>(duration) /
                                                 static_cast<double>(secondsInWeek))) %
                     weeksBetween;
    return wing == 0;
}

WeeklyModifier ForWing(const RaidWing& wing, int secondsInWeek) {
    WeeklyModifier modifier;
    modifier.emboldened =
        IsModifierActive(wing.emboldenedTimestamp, wing.emboldenedWeeks, secondsInWeek);
    modifier.callOfTheMists = IsModifierActive(wing.callOfTheMistsTimestamp,
                                               wing.callOfTheMistsWeeks, secondsInWeek);
    return modifier;
}

}  // namespace WeeklyModifierService
}  // namespace rc
