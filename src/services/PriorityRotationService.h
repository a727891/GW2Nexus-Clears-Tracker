#pragma once

#include <chrono>

namespace rc {

int DayOfYearIndex(std::chrono::system_clock::time_point time =
                       std::chrono::system_clock::now());

}  // namespace rc
