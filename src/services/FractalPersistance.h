#pragma once

#include "data/FractalMapData.h"

#include <chrono>
#include <map>
#include <string>
#include <unordered_map>

namespace rc {

class FractalPersistance {
public:
    std::string version = "3.0.0";
    std::map<std::string, std::map<std::string, std::chrono::system_clock::time_point>>
        accountClears;
    std::unordered_map<std::string, std::string> encounterLabels;
    std::unordered_map<std::string, bool> challengeMoteVisible;

    void Load(const std::string& path, const FractalMapData& fractalData);
    void Save(const std::string& path) const;
    void SaveClear(const std::string& account, const std::string& apiLabel);
    void RemoveClear(const std::string& account, const std::string& apiLabel);
    std::map<std::string, std::chrono::system_clock::time_point> GetClearsForAccount(
        const std::string& account, const FractalMapData& fractalData) const;

    std::string GetEncounterLabel(const std::string& encounterId,
                                  const std::string& defaultAbbrev) const;
    void SetEncounterLabel(const std::string& encounterId, const std::string& label);

    void EnsureChallengeMoteDefaults(const FractalMapData& fractalData);
    bool IsChallengeMoteVisible(const std::string& apiLabel) const;
    void SetChallengeMoteVisible(const std::string& apiLabel, bool visible);
};

}  // namespace rc
