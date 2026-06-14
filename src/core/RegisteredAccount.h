#pragma once

#include <string>
#include <vector>

namespace rc {

struct RegisteredAccount {
    std::string tokenId;
    std::string keyName;
    std::string accountName;
    std::string apiKey;
    std::vector<std::string> characters;
    std::vector<std::string> permissions;
};

struct RegisterKeyResult {
    bool success = false;
    std::string message;
    RegisteredAccount account;
};

}  // namespace rc
