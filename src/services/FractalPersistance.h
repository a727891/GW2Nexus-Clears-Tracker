#pragma once

#include "data/FractalMapData.h"

#include <chrono>
#include <map>
#include <string>

namespace rc {

class FractalPersistance {
public:
    std::string version = "3.0.0";
    std::map<std::string, std::map<std::string, std::chrono::system_clock::time_point>>
        accountClears;

    void Load(const std::string& path, const FractalMapData& fractalData);
    void Save(const std::string& path) const;
    void SaveClear(const std::string& account, const std::string& apiLabel);
    void RemoveClear(const std::string& account, const std::string& apiLabel);
    std::map<std::string, std::chrono::system_clock::time_point> GetClearsForAccount(
        const std::string& account, const FractalMapData& fractalData) const;
};

}  // namespace rc
