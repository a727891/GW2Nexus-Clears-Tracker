#include "services/ApiPollService.h"

namespace rc {

ApiPollService::ApiPollService(int intervalMinutes)
    : intervalSeconds_(static_cast<float>(intervalMinutes) * 60.0f + 3.0f) {}

void ApiPollService::SetIntervalMinutes(int minutes) {
    intervalSeconds_ = static_cast<float>(minutes) * 60.0f + 3.0f;
}

void ApiPollService::SetCallback(Callback cb) { callback_ = std::move(cb); }

void ApiPollService::Update(float deltaSeconds) {
    if (!callback_) return;
    elapsedSeconds_ += deltaSeconds;
    if (elapsedSeconds_ >= intervalSeconds_) {
        elapsedSeconds_ = 0.0f;
        callback_();
    }
}

void ApiPollService::InvokeNow() {
    elapsedSeconds_ = intervalSeconds_;
}

}  // namespace rc
