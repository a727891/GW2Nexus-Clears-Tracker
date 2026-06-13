#include "services/WeeklyBountyEncountersService.h"

#include "services/DailyBountyService.h"
#include "services/PriorityRotationService.h"

#include <ctime>

namespace rc {
namespace {

std::chrono::system_clock::time_point UtcMidnight(std::chrono::system_clock::time_point time) {
    const std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    utc.tm_hour = 0;
    utc.tm_min = 0;
    utc.tm_sec = 0;
#ifdef _WIN32
    return std::chrono::system_clock::from_time_t(_mkgmtime(&utc));
#else
    return std::chrono::system_clock::from_time_t(timegm(&utc));
#endif
}

std::chrono::system_clock::time_point AddUtcDays(std::chrono::system_clock::time_point time, int days) {
    const std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    utc.tm_mday += days;
#ifdef _WIN32
    return std::chrono::system_clock::from_time_t(_mkgmtime(&utc));
#else
    return std::chrono::system_clock::from_time_t(timegm(&utc));
#endif
}

}  // namespace

void WeeklyBountyEncountersService::Rebuild(const DailyBountyData& bountyData,
                                            const ResetsWatcher& resets) {
    std::lock_guard lock(mutex_);
    weeklyBountyApiIds_.clear();
    if (!bountyData.enabled) return;

    const auto today = UtcMidnight(std::chrono::system_clock::now());
    const auto lastDayOfWeek = UtcMidnight(resets.NextWeeklyReset() - std::chrono::hours(24));
    if (today > lastDayOfWeek) return;

    for (auto date = today; date <= lastDayOfWeek; date = AddUtcDays(date, 1)) {
        const int dayIndex = DayOfYearIndex(date);
        for (const auto& apiId : DailyBountyService::GetBountyEncounterApiIdsForDay(bountyData,
                                                                                     dayIndex)) {
            weeklyBountyApiIds_.insert(apiId);
        }
    }
}

bool WeeklyBountyEncountersService::IsWeeklyBounty(const std::string& encounterApiId) const {
    if (encounterApiId.empty()) return false;
    std::lock_guard lock(mutex_);
    return weeklyBountyApiIds_.count(encounterApiId) > 0;
}

}  // namespace rc
