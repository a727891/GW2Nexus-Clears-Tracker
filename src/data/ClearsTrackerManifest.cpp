#include "data/ClearsTrackerManifest.h"

#include "data/StaticDataLoader.h"

#include <nlohmann/json.hpp>

namespace rc {
namespace {

bool NeedsUpdate(const ClearsTrackerManifest& remote,
                 const ClearsTrackerManifest& local,
                 const std::string& remoteVersion,
                 const std::string& localVersion) {
    return remote.cacheBust || remoteVersion != localVersion;
}

bool DownloadIfNeeded(const std::string& addonDir,
                      const char* filename,
                      bool needsUpdate,
                      bool& anyFileUpdated) {
    if (!needsUpdate) return true;

    std::string content;
    if (!StaticDataLoader::Download(addonDir, filename, content)) {
        return false;
    }

    anyFileUpdated = true;
    return true;
}

}  // namespace

bool ClearsTrackerManifest::Parse(const std::string& json, ClearsTrackerManifest& out) {
    try {
        const auto j = nlohmann::json::parse(json);
        out.cacheBust = j.value("cache_bust", false);
        out.raidDataVersion = j.value("raid_data", "");
        out.strikeDataVersion = j.value("strike_data", "");
        out.dailyBountiesVersion = j.value("daily_bounties", "");
        out.fractalMapVersion = j.value("fractal_map_data", "");
        out.instabilitiesVersion = j.value("fractal_instabilities", "");
        out.motd = j.value("motd", "");
        out.motdId = j.value("motd_id", "");
        return true;
    } catch (...) {
        return false;
    }
}

bool ClearsTrackerManifest::LoadLocal(const std::string& addonDir, ClearsTrackerManifest& out) {
    std::string json;
    if (!StaticDataLoader::LoadCached(addonDir, kFilename, json)) {
        out = {};
        return false;
    }
    if (!Parse(json, out)) {
        out = {};
        return false;
    }
    return true;
}

bool ClearsTrackerManifest::DownloadRemote(const std::string& addonDir,
                                           ClearsTrackerManifest& out,
                                           std::string& outRawJson) {
    if (!StaticDataLoader::Download(addonDir, kFilename, outRawJson)) {
        out = {};
        return false;
    }
    if (!Parse(outRawJson, out)) {
        out = {};
        outRawJson.clear();
        return false;
    }
    return true;
}

bool ClearsTrackerManifest::SaveLocal(const std::string& addonDir, const std::string& rawJson) {
    return StaticDataLoader::WriteCached(addonDir, kFilename, rawJson);
}

ClearsTrackerSyncResult ClearsTrackerSync::SyncVersions(const std::string& addonDir) {
    ClearsTrackerSyncResult result;

    ClearsTrackerManifest remote;
    std::string remoteJson;
    if (!ClearsTrackerManifest::DownloadRemote(addonDir, remote, remoteJson)) {
        return result;
    }
    result.remoteManifestOk = true;

    ClearsTrackerManifest local;
    ClearsTrackerManifest::LoadLocal(addonDir, local);

    DownloadIfNeeded(
        addonDir, "fractal_instabilities.json",
        NeedsUpdate(remote, local, remote.instabilitiesVersion, local.instabilitiesVersion),
        result.anyFileUpdated);
    DownloadIfNeeded(addonDir, "fractal_maps.json",
                     NeedsUpdate(remote, local, remote.fractalMapVersion, local.fractalMapVersion),
                     result.anyFileUpdated);
    DownloadIfNeeded(addonDir, "strike_data.json",
                     NeedsUpdate(remote, local, remote.strikeDataVersion, local.strikeDataVersion),
                     result.anyFileUpdated);
    DownloadIfNeeded(addonDir, "raid_data.json",
                     NeedsUpdate(remote, local, remote.raidDataVersion, local.raidDataVersion),
                     result.anyFileUpdated);
    DownloadIfNeeded(
        addonDir, "daily_bounties.json",
        NeedsUpdate(remote, local, remote.dailyBountiesVersion, local.dailyBountiesVersion),
        result.anyFileUpdated);

    ClearsTrackerManifest::SaveLocal(addonDir, remoteJson);
    return result;
}

}  // namespace rc
