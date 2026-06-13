#include "data/DailyBountyData.h"

namespace rc {

DailyBountyData DailyBountyData::FromJson(const nlohmann::json& j) {
    DailyBountyData data;
    data.enabled = j.value("enabled", false);
    data.version = j.value("version", "");
    data.name = j.value("name", data.name);
    data.abbreviation = j.value("abbreviation", data.abbreviation);
    data.dailyBountyCategoryUrl = j.value("dailyBountyCategoryUrl", data.dailyBountyCategoryUrl);

    if (j.contains("priority_tomorrow")) {
        const auto& t = j["priority_tomorrow"];
        data.tomorrowName = t.value("name", data.tomorrowName);
        data.tomorrowAbbreviation = t.value("abbriviation", data.tomorrowAbbreviation);
    }

    if (j.contains("bossSlots")) {
        for (const auto& slotJ : j["bossSlots"]) {
            BossSlotRotation slot;
            slot.slot = slotJ.value("slot", 0);
            slot.offset = slotJ.value("offset", 0);
            if (slotJ.contains("encounters")) {
                for (const auto& enc : slotJ["encounters"]) {
                    slot.encounters.push_back(enc.get<std::string>());
                }
            }
            data.bossSlots.push_back(std::move(slot));
        }
    }
    return data;
}

}  // namespace rc
