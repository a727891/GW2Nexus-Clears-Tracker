#pragma once

#include "data/RaidData.h"

#include <string>
#include <unordered_map>

namespace rc {

class RaidVisibilityStore {
public:
    std::string version = "1.0.0";
    std::unordered_map<std::string, bool> expansions;
    std::unordered_map<std::string, bool> wings;
    std::unordered_map<std::string, bool> encounters;
    std::unordered_map<std::string, std::string> encounterLabels;

    void Load(const std::string& path);
    void Save(const std::string& path) const;
    void InitializeFromData(const RaidData& data);
    void MigratePriorityLabelKeys();

    std::string GetEncounterLabel(const std::string& encounterId,
                                  const std::string& defaultAbbrev) const;
    void SetEncounterLabel(const std::string& encounterId, const std::string& label);

    bool IsExpansionVisible(const std::string& expansionId) const;
    bool IsWingVisible(const std::string& wingId) const;
    bool IsEncounterVisible(const std::string& encounterId) const;

    void SetExpansionVisible(const std::string& expansionId, bool visible, const RaidData& data);
    void SetWingVisible(const std::string& wingId, bool visible);
    void SetEncounterVisible(const std::string& encounterId, bool visible);
};

}  // namespace rc
