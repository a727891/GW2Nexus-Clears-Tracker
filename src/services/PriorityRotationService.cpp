#include "services/PriorityRotationService.h"

#include <ctime>

namespace rc {

int DayOfYearIndex(std::chrono::system_clock::time_point time) {
    const std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif

    int day = utc.tm_yday;
    const int year = utc.tm_year + 1900;
    const bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (!isLeap && utc.tm_mon >= 2) {
        day += 1;
    }
    return day;
}

}  // namespace rc
