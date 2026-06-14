#include "data/StaticDataLoader.h"

#include "services/HttpClient.h"

#include <filesystem>
#include <fstream>

namespace rc {

bool StaticDataLoader::EnsureCacheDir(const std::string& addonDir) {
    const auto cachePath = std::filesystem::path(addonDir) / kCacheDirName;
    std::error_code ec;
    std::filesystem::create_directories(cachePath, ec);
    return !ec;
}

bool StaticDataLoader::LoadCached(const std::string& addonDir,
                                  const std::string& filename,
                                  std::string& outContent) {
    const auto cacheFile = std::filesystem::path(addonDir) / kCacheDirName / filename;
    if (!std::filesystem::exists(cacheFile)) return false;

    std::ifstream in(cacheFile);
    if (!in.is_open()) return false;

    outContent.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return !outContent.empty();
}

bool StaticDataLoader::Download(const std::string& addonDir,
                                const std::string& filename,
                                std::string& outContent) {
    EnsureCacheDir(addonDir);
    const auto cacheFile = std::filesystem::path(addonDir) / kCacheDirName / filename;
    const std::string url = std::string(kStaticHostUrl) + filename;

    const auto body = HttpGetUrl(url);
    if (!body || body->empty()) return false;

    outContent = *body;
    std::ofstream out(cacheFile, std::ios::binary);
    out << outContent;
    return true;
}

bool StaticDataLoader::LoadOrDownload(const std::string& addonDir,
                                      const std::string& filename,
                                      std::string& outContent) {
    if (LoadCached(addonDir, filename, outContent)) return true;
    return Download(addonDir, filename, outContent);
}

bool StaticDataLoader::DownloadToPath(const std::filesystem::path& destFile,
                                      const std::string& filename) {
    const std::string url = std::string(kStaticHostUrl) + filename;

    const auto body = HttpGetUrl(url);
    if (!body || body->empty()) return false;

    if (destFile.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(destFile.parent_path(), ec);
    }

    std::ofstream out(destFile, std::ios::binary);
    if (!out.is_open()) return false;
    out << *body;
    return true;
}

}  // namespace rc
