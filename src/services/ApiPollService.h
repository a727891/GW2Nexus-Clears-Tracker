#pragma once

#include <chrono>
#include <functional>

namespace rc {

class ApiPollService {
public:
    using Callback = std::function<void()>;

    explicit ApiPollService(int intervalMinutes = 5);

    void SetIntervalMinutes(int minutes);
    void SetCallback(Callback cb);
    void Update(float deltaSeconds);
    void InvokeNow();

private:
    Callback callback_;
    float intervalSeconds_;
    float elapsedSeconds_ = 0.0f;
};

}  // namespace rc
