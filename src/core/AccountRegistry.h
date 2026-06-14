#pragma once

#include "core/RegisteredAccount.h"
#include "services/Gw2ApiClient.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace rc {

class AccountRegistry {
public:
    void Load(const std::string& path);
    void Save(const std::string& path) const;
    void SetStoragePath(const std::string& path);

    const std::vector<RegisteredAccount>& Accounts() const;
    std::vector<RegisteredAccount> AccountsSnapshot() const;
    std::optional<std::string> ActiveTokenId() const;
    std::optional<std::string> ActiveApiKey() const;
    std::optional<std::string> ActiveAccountName() const;
    std::string CharacterName() const;
    std::string LastResolvedCharacter() const;
    bool HasActiveAccount() const;

    RegisterKeyResult RegisterKey(Gw2ApiClient& client, const std::string& apiKey);
    bool RemoveKey(const std::string& tokenId);

    bool ResolveActiveAccountFromCache(const std::string& characterName);
    bool ResolveActiveAccountWithNetwork(const std::string& characterName, Gw2ApiClient& client);

    void RegisterKeyAsync(const std::string& apiKey,
                          const std::string& savePath,
                          std::function<void(RegisterKeyResult)> onComplete);

    bool IsRegistering() const { return registering_.load(); }
    std::string LastRegistrationMessage() const;

private:
    void RebuildCharacterLookup();
    const RegisteredAccount* FindByTokenId(const std::string& tokenId) const;
    RegisteredAccount* FindByTokenIdMutable(const std::string& tokenId);
    bool SetActiveToken(const std::string& tokenId);

    mutable std::mutex mutex_;
    std::vector<RegisteredAccount> accounts_;
    std::unordered_map<std::string, std::string> characterToTokenId_;
    std::optional<std::string> activeTokenId_;
    std::string characterName_;
    std::string lastResolvedCharacter_;

    std::atomic<bool> registering_{false};
    std::string lastRegistrationMessage_;
    std::string storagePath_;
};

}  // namespace rc
