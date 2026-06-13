#include "services/ResetsWatcher.h"

#include <ctime>

namespace rc {

namespace {

std::chrono::system_clock::time_point UtcFromTm(std::tm tm) {
    tm.tm_isdst = 0;
#ifdef _WIN32
    const std::time_t tt = _mkgmtime(&tm);
#else
    const std::time_t tt = timegm(&tm);
#endif
    return std::chrono::system_clock::from_time_t(tt);
}

std::chrono::system_clock::time_point NowUtc() {
    return std::chrono::system_clock::now();
}

}  // namespace

ResetsWatcher::ResetsWatcher() {
    CalcNextDailyReset();
    CalcNextWeeklyReset();
}

void ResetsWatcher::CalcNextDailyReset() {
    const auto now = NowUtc();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif

    std::tm next = utc;
    next.tm_mday += 1;
    next.tm_hour = 0;
    next.tm_min = 0;
    next.tm_sec = 0;
    nextDailyReset_ = UtcFromTm(next);
    lastDailyReset_ = nextDailyReset_ - std::chrono::hours(24);
}

std::chrono::system_clock::time_point ResetsWatcher::NextMondayReset() {
    const auto now = NowUtc();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif

    std::tm target = utc;
    if (utc.tm_wday == 1 && utc.tm_hour < 7) {
        target.tm_hour = 7;
        target.tm_min = 30;
        target.tm_sec = 0;
        return UtcFromTm(target);
    }

    int daysUntilMonday = (8 - utc.tm_wday) % 7;
    if (daysUntilMonday == 0) daysUntilMonday = 7;
    target.tm_mday += daysUntilMonday;
    target.tm_hour = 7;
    target.tm_min = 30;
    target.tm_sec = 0;
    return UtcFromTm(target);
}

void ResetsWatcher::CalcNextWeeklyReset() {
    nextWeeklyReset_ = NextMondayReset();
    lastWeeklyReset_ = nextWeeklyReset_ - std::chrono::hours(24 * 7);
}

void ResetsWatcher::Update() {
    const auto now = NowUtc();
    if (now >= nextDailyReset_) {
        CalcNextDailyReset();
    }
    if (now >= nextWeeklyReset_) {
        CalcNextWeeklyReset();
    }
}

}  // namespace rc
