#pragma once

#include <chrono>

namespace rc {

class ResetsWatcher {
public:
    ResetsWatcher();

    void Update();
    std::chrono::system_clock::time_point LastDailyReset() const { return lastDailyReset_; }
    std::chrono::system_clock::time_point LastWeeklyReset() const { return lastWeeklyReset_; }
    std::chrono::system_clock::time_point NextDailyReset() const { return nextDailyReset_; }
    std::chrono::system_clock::time_point NextWeeklyReset() const { return nextWeeklyReset_; }

private:
    void CalcNextDailyReset();
    void CalcNextWeeklyReset();
    static std::chrono::system_clock::time_point NextMondayReset();

    std::chrono::system_clock::time_point lastDailyReset_;
    std::chrono::system_clock::time_point nextDailyReset_;
    std::chrono::system_clock::time_point lastWeeklyReset_;
    std::chrono::system_clock::time_point nextWeeklyReset_;
};

}  // namespace rc
