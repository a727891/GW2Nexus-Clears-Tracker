#pragma once

#include "data/StrikeData.h"
#include "services/Gw2ApiClient.h"
#include "services/StrikePersistance.h"

namespace rc {

class StrikeClearsService {
public:
    static void RefreshFromApi(const StrikeData& strikeData,
                               const std::string& accountName,
                               Gw2ApiClient& client,
                               StrikePersistance& persistence);
};

}  // namespace rc
