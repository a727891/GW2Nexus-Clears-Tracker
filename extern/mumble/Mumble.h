#pragma once

#include <cstdint>

namespace Mumble {

#pragma pack(push, 1)
struct Context {
    unsigned char serverAddress[28];
    uint32_t mapId;
    uint32_t mapType;
    uint32_t shardId;
    uint32_t instance;
    uint32_t buildId;
    uint32_t uiState;
    uint16_t compassWidth;
    uint16_t compassHeight;
    float compassRotation;
    float playerX;
    float playerY;
    float mapCenterX;
    float mapCenterY;
    float mapScale;
    uint32_t processId;
    uint8_t mountIndex;
};
#pragma pack(pop)

struct LinkedMem {
    uint32_t uiVersion;
    uint32_t uiTick;
    float fAvatarPosition[3];
    float fAvatarFront[3];
    float fAvatarTop[3];
    wchar_t name[256];
    float fCameraPosition[3];
    float fCameraFront[3];
    float fCameraTop[3];
    wchar_t identity[256];
    uint32_t context_len;
    unsigned char context[256];
    wchar_t description[2048];
};

struct Data {
    LinkedMem LinkedMemory;
};

inline bool IsMapOpen(const Data* data) {
    if (!data) return false;
    const auto* ctx = reinterpret_cast<const Context*>(data->LinkedMemory.context);
    return (ctx->uiState & 1u) != 0;
}

inline uint32_t GetMapId(const Data* data) {
    if (!data) return 0;
    const auto* ctx = reinterpret_cast<const Context*>(data->LinkedMemory.context);
    return ctx->mapId;
}

}  // namespace Mumble
