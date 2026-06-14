#pragma once

#include <string>

namespace rc {

struct ClearsTrackerManifest {
    bool cacheBust = false;
    std::string raidDataVersion;
    std::string strikeDataVersion;
    std::string dailyBountiesVersion;
    std::string fractalMapVersion;
    std::string instabilitiesVersion;
    std::string motd;
    std::string motdId;

    static constexpr const char* kFilename = "clears_tracker.json";

    static bool Parse(const std::string& json, ClearsTrackerManifest& out);
    static bool LoadLocal(const std::string& addonDir, ClearsTrackerManifest& out);
    static bool DownloadRemote(const std::string& addonDir,
                               ClearsTrackerManifest& out,
                               std::string& outRawJson);
    static bool SaveLocal(const std::string& addonDir, const std::string& rawJson);
};

struct ClearsTrackerSyncResult {
    bool remoteManifestOk = false;
    bool anyFileUpdated = false;
};

class ClearsTrackerSync {
public:
    static ClearsTrackerSyncResult SyncVersions(const std::string& addonDir);
};

}  // namespace rc
