#include "services/FractalPersistance.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

namespace rc {

namespace {

std::string TimeToString(std::chrono::system_clock::time_point tp) {
    const std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    std::ostringstream oss;
    oss << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::chrono::system_clock::time_point TimeFromString(const std::string& s) {
    if (s.empty()) return {};
    std::tm tm{};
    std::istringstream iss(s);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (iss.fail()) return {};
#ifdef _WIN32
    const std::time_t tt = _mkgmtime(&tm);
#else
    const std::time_t tt = timegm(&tm);
#endif
    return std::chrono::system_clock::from_time_t(tt);
}

std::map<std::string, std::chrono::system_clock::time_point> EmptyClears(
    const FractalMapData& fractalData) {
    std::map<std::string, std::chrono::system_clock::time_point> clears;
    for (const auto& [_, map] : fractalData.maps) {
        if (map.IsValid()) {
            clears[map.apiLabel] = {};
        }
    }
    return clears;
}

}  // namespace

void FractalPersistance::Load(const std::string& path, const FractalMapData& fractalData) {
    std::ifstream in(path);
    if (!in.is_open()) {
        accountClears.clear();
        return;
    }

    nlohmann::json j;
    in >> j;
    version = j.value("version", version);
    accountClears.clear();
    encounterLabels.clear();
    if (j.contains("encounterLabels") && j["encounterLabels"].is_object()) {
        for (const auto& [id, label] : j["encounterLabels"].items()) {
            if (label.is_string()) {
                encounterLabels[id] = label.get<std::string>();
            }
        }
    }
    if (!j.contains("accountClears")) return;

    for (auto it = j["accountClears"].begin(); it != j["accountClears"].end(); ++it) {
        auto clears = EmptyClears(fractalData);
        for (auto cit = it.value().begin(); cit != it.value().end(); ++cit) {
            clears[cit.key()] = TimeFromString(cit.value().get<std::string>());
        }
        accountClears[it.key()] = std::move(clears);
    }
}

void FractalPersistance::Save(const std::string& path) const {
    nlohmann::json j;
    j["version"] = version;
    nlohmann::json accounts = nlohmann::json::object();
    for (const auto& [account, clears] : accountClears) {
        nlohmann::json c = nlohmann::json::object();
        for (const auto& [id, time] : clears) {
            c[id] = TimeToString(time);
        }
        accounts[account] = c;
    }
    j["accountClears"] = accounts;

    nlohmann::json labels = nlohmann::json::object();
    for (const auto& [id, label] : encounterLabels) {
        labels[id] = label;
    }
    j["encounterLabels"] = labels;

    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }
    std::ofstream out(path);
    out << j.dump(2);
}

void FractalPersistance::SaveClear(const std::string& account, const std::string& apiLabel) {
    accountClears[account][apiLabel] = std::chrono::system_clock::now();
}

void FractalPersistance::RemoveClear(const std::string& account, const std::string& apiLabel) {
    accountClears[account][apiLabel] = {};
}

std::map<std::string, std::chrono::system_clock::time_point>
FractalPersistance::GetClearsForAccount(const std::string& account,
                                        const FractalMapData& fractalData) const {
    if (auto it = accountClears.find(account); it != accountClears.end()) {
        return it->second;
    }
    return EmptyClears(fractalData);
}

std::string FractalPersistance::GetEncounterLabel(const std::string& encounterId,
                                                const std::string& defaultAbbrev) const {
    if (const auto it = encounterLabels.find(encounterId); it != encounterLabels.end()) {
        return it->second;
    }
    return defaultAbbrev;
}

void FractalPersistance::SetEncounterLabel(const std::string& encounterId,
                                          const std::string& label) {
    encounterLabels[encounterId] = label;
}

}  // namespace rc
