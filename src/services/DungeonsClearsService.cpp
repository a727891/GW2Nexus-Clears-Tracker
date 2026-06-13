#include "services/DungeonsClearsService.h"

namespace rc {

std::unordered_set<std::string> DungeonsClearsService::FetchWeeklyClears(Gw2ApiClient& api) {
    if (auto clears = api.FetchDungeonClears()) {
        return *clears;
    }
    return {};
}

std::unordered_set<std::string> DungeonsClearsService::FetchFrequenterPaths(Gw2ApiClient& api) {
    if (auto paths = api.FetchFrequenterPaths()) {
        return *paths;
    }
    return {};
}

std::string DungeonsClearsService::FrequenterBitToPathId(int bit) {
    switch (bit) {
        case 0:
            return "coe_story";
        case 1:
            return "submarine";
        case 2:
            return "teleporter";
        case 3:
            return "front_door";
        case 4:
            return "ac_story";
        case 5:
            return "hodgins";
        case 6:
            return "detha";
        case 7:
            return "tzark";
        case 8:
            return "jotun";
        case 9:
            return "mursaat";
        case 10:
            return "forgotten";
        case 11:
            return "seer";
        case 12:
            return "cm_story";
        case 13:
            return "asura";
        case 14:
            return "seraph";
        case 15:
            return "butler";
        case 16:
            return "se_story";
        case 17:
            return "fergg";
        case 18:
            return "rasalov";
        case 19:
            return "koptev";
        case 20:
            return "ta_story";
        case 21:
            return "leurent";
        case 22:
            return "vevina";
        case 23:
            return "aetherpath";
        case 24:
            return "hotw_story";
        case 25:
            return "butcher";
        case 26:
            return "plunderer";
        case 27:
            return "zealot";
        case 28:
            return "cof_story";
        case 29:
            return "ferrah";
        case 30:
            return "magg";
        case 31:
            return "rhiannon";
        default:
            return {};
    }
}

}  // namespace rc
