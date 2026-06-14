#include "data/StaticDataLoader.h"

#include "services/HttpClient.h"

#include <filesystem>
#include <fstream>

namespace rc {

namespace {

bool WriteCacheFile(const std::filesystem::path& cacheFile, const std::string& body) {
    if (cacheFile.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(cacheFile.parent_path(), ec);
    }

    std::ofstream out(cacheFile, std::ios::binary);
    if (!out.is_open()) return false;
    out << body;
    return true;
}

bool DownloadFromUrl(const std::string& baseUrl,
                     const std::filesystem::path& cacheFile,
                     std::string& outContent) {
    const auto filename = cacheFile.filename().string();
    const std::string url = baseUrl + filename;

    const auto body = HttpGetUrl(url);
    if (!body || body->empty()) return false;

    outContent = *body;
    return WriteCacheFile(cacheFile, outContent);
}

}  // namespace

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

bool StaticDataLoader::WriteCached(const std::string& addonDir,
                                   const std::string& filename,
                                   const std::string& content) {
    EnsureCacheDir(addonDir);
    const auto cacheFile = std::filesystem::path(addonDir) / kCacheDirName / filename;
    return WriteCacheFile(cacheFile, content);
}

bool StaticDataLoader::Download(const std::string& addonDir,
                                const std::string& filename,
                                std::string& outContent) {
    EnsureCacheDir(addonDir);
    const auto cacheFile = std::filesystem::path(addonDir) / kCacheDirName / filename;
    return DownloadFromUrl(kStaticHostUrl, cacheFile, outContent);
}

bool StaticDataLoader::LoadOrDownload(const std::string& addonDir,
                                      const std::string& filename,
                                      std::string& outContent) {
    if (LoadCached(addonDir, filename, outContent)) return true;
    return Download(addonDir, filename, outContent);
}

}  // namespace rc
