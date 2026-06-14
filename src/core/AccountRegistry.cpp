#include "core/AccountRegistry.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>

namespace rc {
namespace {

constexpr const char* kCurrentVersion = "1.0.0";

bool HasPermission(const std::vector<std::string>& permissions, const char* perm) {
    return std::find(permissions.begin(), permissions.end(), perm) != permissions.end();
}

RegisteredAccount AccountFromJson(const nlohmann::json& j) {
    RegisteredAccount account;
    account.tokenId = j.value("tokenId", "");
    account.keyName = j.value("keyName", "");
    account.accountName = j.value("accountName", "");
    account.apiKey = j.value("apiKey", "");
    if (j.contains("characters") && j["characters"].is_array()) {
        for (const auto& c : j["characters"]) {
            if (c.is_string()) {
                const auto name = c.get<std::string>();
                if (!name.empty() && name != "not found") {
                    account.characters.push_back(name);
                }
            }
        }
    }
    if (j.contains("permissions") && j["permissions"].is_array()) {
        for (const auto& p : j["permissions"]) {
            if (p.is_string()) {
                account.permissions.push_back(p.get<std::string>());
            }
        }
    }
    return account;
}

nlohmann::json AccountToJson(const RegisteredAccount& account) {
    return {{"tokenId", account.tokenId},
            {"keyName", account.keyName},
            {"accountName", account.accountName},
            {"apiKey", account.apiKey},
            {"characters", account.characters},
            {"permissions", account.permissions}};
}

RegisterKeyResult BuildProfile(Gw2ApiClient& client, const std::string& apiKey) {
    RegisterKeyResult result;
    if (apiKey.empty()) {
        result.message = "API key is empty.";
        return result;
    }

    client.SetApiKey(apiKey);
    const auto tokenInfo = client.FetchTokenInfo();
    if (!tokenInfo.valid) {
        result.message = "API key validation failed.";
        return result;
    }

    if (!HasPermission(tokenInfo.permissions, "account") ||
        !HasPermission(tokenInfo.permissions, "progression") ||
        !HasPermission(tokenInfo.permissions, "characters")) {
        result.message =
            "API key must have account, progression, and characters permissions.";
        return result;
    }

    const auto accountName = client.FetchAccountName();
    if (!accountName || accountName->empty()) {
        result.message = "Failed to fetch account name.";
        return result;
    }

    const auto characters = client.FetchAccountCharacters();
    if (!characters) {
        result.message = "Failed to fetch account characters.";
        return result;
    }

    RegisteredAccount account;
    account.tokenId = tokenInfo.id;
    account.keyName = tokenInfo.name;
    account.accountName = *accountName;
    account.apiKey = apiKey;
    account.characters = *characters;
    account.permissions = tokenInfo.permissions;

    result.success = true;
    result.message = "Registered " + account.accountName + ".";
    result.account = std::move(account);
    return result;
}

}  // namespace

void AccountRegistry::Load(const std::string& path) {
    std::lock_guard lock(mutex_);
    accounts_.clear();
    characterToTokenId_.clear();
    activeTokenId_.reset();

    std::ifstream in(path);
    if (!in.is_open()) return;

    nlohmann::json j;
    in >> j;
    if (!j.contains("accounts") || !j["accounts"].is_array()) return;

    for (const auto& entry : j["accounts"]) {
        auto account = AccountFromJson(entry);
        if (!account.tokenId.empty() && !account.apiKey.empty()) {
            accounts_.push_back(std::move(account));
        }
    }
    RebuildCharacterLookup();
}

void AccountRegistry::Save(const std::string& path) const {
    nlohmann::json j;
    j["version"] = kCurrentVersion;
    j["accounts"] = nlohmann::json::array();

    {
        std::lock_guard lock(mutex_);
        for (const auto& account : accounts_) {
            j["accounts"].push_back(AccountToJson(account));
        }
    }

    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    std::ofstream out(path);
    out << j.dump(2);
}

void AccountRegistry::SetStoragePath(const std::string& path) {
    std::lock_guard lock(mutex_);
    storagePath_ = path;
}

const std::vector<RegisteredAccount>& AccountRegistry::Accounts() const {
    return accounts_;
}

std::vector<RegisteredAccount> AccountRegistry::AccountsSnapshot() const {
    std::lock_guard lock(mutex_);
    return accounts_;
}

std::optional<std::string> AccountRegistry::ActiveTokenId() const {
    std::lock_guard lock(mutex_);
    return activeTokenId_;
}

std::optional<std::string> AccountRegistry::ActiveApiKey() const {
    std::lock_guard lock(mutex_);
    if (!activeTokenId_) return std::nullopt;
    const auto* account = FindByTokenId(*activeTokenId_);
    if (!account) return std::nullopt;
    return account->apiKey;
}

std::optional<std::string> AccountRegistry::ActiveAccountName() const {
    std::lock_guard lock(mutex_);
    if (!activeTokenId_) return std::nullopt;
    const auto* account = FindByTokenId(*activeTokenId_);
    if (!account) return std::nullopt;
    return account->accountName;
}

std::string AccountRegistry::CharacterName() const {
    std::lock_guard lock(mutex_);
    return characterName_;
}

std::string AccountRegistry::LastResolvedCharacter() const {
    std::lock_guard lock(mutex_);
    return lastResolvedCharacter_;
}

bool AccountRegistry::HasActiveAccount() const {
    std::lock_guard lock(mutex_);
    return activeTokenId_.has_value();
}

std::string AccountRegistry::LastRegistrationMessage() const {
    std::lock_guard lock(mutex_);
    return lastRegistrationMessage_;
}

void AccountRegistry::RebuildCharacterLookup() {
    characterToTokenId_.clear();
    for (const auto& account : accounts_) {
        for (const auto& character : account.characters) {
            characterToTokenId_[character] = account.tokenId;
        }
    }
}

const RegisteredAccount* AccountRegistry::FindByTokenId(const std::string& tokenId) const {
    for (const auto& account : accounts_) {
        if (account.tokenId == tokenId) return &account;
    }
    return nullptr;
}

RegisteredAccount* AccountRegistry::FindByTokenIdMutable(const std::string& tokenId) {
    for (auto& account : accounts_) {
        if (account.tokenId == tokenId) return &account;
    }
    return nullptr;
}

bool AccountRegistry::SetActiveToken(const std::string& tokenId) {
    if (!FindByTokenId(tokenId)) return false;
    activeTokenId_ = tokenId;
    return true;
}

RegisterKeyResult AccountRegistry::RegisterKey(Gw2ApiClient& client, const std::string& apiKey) {
    auto result = BuildProfile(client, apiKey);
    if (!result.success) {
        std::lock_guard lock(mutex_);
        lastRegistrationMessage_ = result.message;
        return result;
    }

    std::lock_guard lock(mutex_);
    if (auto* existing = FindByTokenIdMutable(result.account.tokenId)) {
        *existing = result.account;
    } else {
        accounts_.push_back(result.account);
    }
    RebuildCharacterLookup();
    lastRegistrationMessage_ = result.message;
    return result;
}

bool AccountRegistry::RemoveKey(const std::string& tokenId) {
    std::lock_guard lock(mutex_);
    const auto it = std::remove_if(accounts_.begin(), accounts_.end(),
                                   [&](const RegisteredAccount& a) { return a.tokenId == tokenId; });
    if (it == accounts_.end()) return false;

    accounts_.erase(it, accounts_.end());
    if (activeTokenId_ && *activeTokenId_ == tokenId) {
        activeTokenId_.reset();
    }
    RebuildCharacterLookup();
    return true;
}

bool AccountRegistry::ResolveActiveAccountFromCache(const std::string& characterName) {
    std::lock_guard lock(mutex_);
    characterName_ = characterName;
    lastResolvedCharacter_ = characterName;

    if (characterName.empty()) {
        const bool changed = activeTokenId_.has_value();
        activeTokenId_.reset();
        return changed;
    }

    if (const auto it = characterToTokenId_.find(characterName);
        it != characterToTokenId_.end()) {
        if (FindByTokenId(it->second)) {
            const bool changed = !activeTokenId_ || *activeTokenId_ != it->second;
            activeTokenId_ = it->second;
            return changed;
        }
    }

    const bool changed = activeTokenId_.has_value();
    activeTokenId_.reset();
    return changed;
}

bool AccountRegistry::ResolveActiveAccountWithNetwork(const std::string& characterName,
                                                      Gw2ApiClient& client) {
    std::vector<RegisteredAccount> accountsCopy;
    std::string savePath;
    {
        std::lock_guard lock(mutex_);
        characterName_ = characterName;
        lastResolvedCharacter_ = characterName;
        accountsCopy = accounts_;
        savePath = storagePath_;
    }

    if (characterName.empty()) {
        std::lock_guard lock(mutex_);
        const bool changed = activeTokenId_.has_value();
        activeTokenId_.reset();
        return changed;
    }

    bool charactersUpdated = false;
    bool matchedAccount = false;
    bool matchedChanged = false;

    for (auto& account : accountsCopy) {
        client.SetApiKey(account.apiKey);
        const auto characters = client.FetchAccountCharacters();
        if (!characters) continue;

        account.characters = *characters;
        bool matched = false;
        for (const auto& name : account.characters) {
            if (name == characterName) {
                matched = true;
                break;
            }
        }

        {
            std::lock_guard lock(mutex_);
            if (auto* stored = FindByTokenIdMutable(account.tokenId)) {
                stored->characters = account.characters;
                RebuildCharacterLookup();
                charactersUpdated = true;
            }

            if (matched) {
                matchedChanged = !activeTokenId_ || *activeTokenId_ != account.tokenId;
                activeTokenId_ = account.tokenId;
                matchedAccount = true;
                break;
            }
        }
    }

    if (charactersUpdated && !savePath.empty()) {
        Save(savePath);
    }

    if (matchedAccount) {
        return matchedChanged;
    }

    std::lock_guard lock(mutex_);
    const bool changed = activeTokenId_.has_value();
    activeTokenId_.reset();
    return changed;
}

void AccountRegistry::RegisterKeyAsync(const std::string& apiKey,
                                       const std::string& savePath,
                                       std::function<void(RegisterKeyResult)> onComplete) {
    if (registering_.exchange(true)) {
        if (onComplete) {
            RegisterKeyResult result;
            result.message = "Registration already in progress.";
            onComplete(result);
        }
        return;
    }

    std::thread([this, apiKey, savePath, onComplete]() {
        Gw2ApiClient threadClient;
        const auto result = RegisterKey(threadClient, apiKey);
        if (result.success) {
            Save(savePath);
        }
        registering_.store(false);
        if (onComplete) {
            onComplete(result);
        }
    }).detach();
}

}  // namespace rc
