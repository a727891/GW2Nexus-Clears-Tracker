#pragma once

#include "mumble/Mumble.h"

#include <string>

namespace rc {
namespace MumbleIdentity {

std::string WideToUtf8(const wchar_t* wide);
std::string ParseIdentityName(const Mumble::Data* data);

}  // namespace MumbleIdentity
}  // namespace rc
