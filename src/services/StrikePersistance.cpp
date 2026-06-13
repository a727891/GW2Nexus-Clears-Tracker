#include "services/StrikePersistance.h"

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
    const StrikeData& strikeData) {
    std::map<std::string, std::chrono::system_clock::time_point> clears;
    for (const auto& exp : strikeData.expansions) {
        for (const auto& m : exp.missions) {
            clears[m.id] = {};
        }
    }
    return clears;
}

}  // namespace

void StrikePersistance::Load(const std::string& path, const StrikeData& strikeData) {
    std::ifstream in(path);
    if (!in.is_open()) {
        accountClears.clear();
        return;
    }

    nlohmann::json j;
    in >> j;
    version = j.value("version", version);
    accountClears.clear();

    if (!j.contains("accountClears")) return;
    for (auto it = j["accountClears"].begin(); it != j["accountClears"].end(); ++it) {
        std::map<std::string, std::chrono::system_clock::time_point> clears = EmptyClears(strikeData);
        for (auto cit = it.value().begin(); cit != it.value().end(); ++cit) {
            clears[cit.key()] = TimeFromString(cit.value().get<std::string>());
        }
        accountClears[it.key()] = std::move(clears);
    }
}

void StrikePersistance::Save(const std::string& path) const {
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

    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }
    std::ofstream out(path);
    out << j.dump(2);
}

void StrikePersistance::SaveClear(const std::string& account, const std::string& encounterId) {
    auto& clears = accountClears[account];
    clears[encounterId] = std::chrono::system_clock::now();
}

void StrikePersistance::RemoveClear(const std::string& account, const std::string& encounterId) {
    auto& clears = accountClears[account];
    clears[encounterId] = {};
}

std::map<std::string, std::chrono::system_clock::time_point> StrikePersistance::GetClearsForAccount(
    const std::string& account, const StrikeData& strikeData) const {
    auto it = accountClears.find(account);
    if (it == accountClears.end()) {
        return EmptyClears(strikeData);
    }
    return it->second;
}

}  // namespace rc
