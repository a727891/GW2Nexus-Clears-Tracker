#pragma once

#include <filesystem>
#include <string>

namespace rc {

class StaticDataLoader {
public:
    static constexpr const char* kStaticHostUrl =
        "https://bhm.blishhud.com/Soeed.RaidClears/static/v2/";
    static constexpr const char* kCacheDirName = "clearsTracker";

    static bool EnsureCacheDir(const std::string& addonDir);
    static bool LoadCached(const std::string& addonDir,
                           const std::string& filename,
                           std::string& outContent);
    static bool WriteCached(const std::string& addonDir,
                            const std::string& filename,
                            const std::string& content);
    static bool Download(const std::string& addonDir,
                         const std::string& filename,
                         std::string& outContent);
    static bool LoadOrDownload(const std::string& addonDir,
                               const std::string& filename,
                               std::string& outContent);
};

}  // namespace rc
