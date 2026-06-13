#pragma once

#include "data/StrikeData.h"
#include <chrono>
#include <map>
#include <string>

namespace rc {

class StrikePersistance {
public:
    std::string version = "3.0.0";
    std::map<std::string, std::map<std::string, std::chrono::system_clock::time_point>> accountClears;

    void Load(const std::string& path, const StrikeData& strikeData);
    void Save(const std::string& path) const;
    void SaveClear(const std::string& account, const std::string& encounterId);
    void RemoveClear(const std::string& account, const std::string& encounterId);
    std::map<std::string, std::chrono::system_clock::time_point> GetClearsForAccount(
        const std::string& account, const StrikeData& strikeData) const;
};

}  // namespace rc
