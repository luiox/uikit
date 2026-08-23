#include "backend.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <ShObjIdl.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <atlbase.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <vector>

#include <wincrypt.h>

#include <nlohmann/json.hpp>

#include "logger.h"
#include "utils/string_util.h"

namespace backend {

using nlohmann::json;

namespace {

struct LegacyItem {
    std::string name;
    std::string target_path;
    std::string icon_location;
    std::string arguments;
    std::uint64_t count = 0;
};

void SetError(std::string* out, const std::string& msg) {
    if (out) {
        *out = msg;
    }
}

std::string ReadTextFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool WriteTextAtomic(const std::filesystem::path& path, const std::string& content, std::string* error) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    const auto tmp = path.string() + ".tmp";

    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) {
            SetError(error, "open temp file failed: " + tmp);
            return false;
        }
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        if (!out.good()) {
            SetError(error, "write temp file failed: " + tmp);
            return false;
        }
    }

    std::filesystem::rename(tmp, path, ec);
    if (!ec) {
        return true;
    }

    std::filesystem::remove(path, ec);
    ec.clear();
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        SetError(error, "replace file failed: " + path.string());
        return false;
    }
    return true;
}

bool BackupCorruptedJson(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }

    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    const auto backup_path = path.string() + ".bad." + std::to_string(timestamp) + ".bak";

    std::error_code ec;
    std::filesystem::copy_file(path, backup_path, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        launcher::log::Warn("backup failed for corrupted json: " + path.string() + " error=" + ec.message());
        return false;
    }

    launcher::log::Warn("backup corrupted json: " + path.string() + " -> " + backup_path);
    return true;
}

std::tm ToLocalTime(std::chrono::system_clock::time_point tp) {
    const auto time_t_value = std::chrono::system_clock::to_time_t(tp);
    std::tm local{};
    localtime_s(&local, &time_t_value);
    return local;
}

std::string FormatJournalTimestamp(std::chrono::system_clock::time_point tp) {
    const auto local = ToLocalTime(tp);
    char buffer[32]{};
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
        local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
        local.tm_hour, local.tm_min, local.tm_sec);
    return buffer;
}

std::string FormatFileStamp(std::chrono::system_clock::time_point tp) {
    const auto local = ToLocalTime(tp);
    char buffer[32]{};
    std::snprintf(buffer, sizeof(buffer), "%04d%02d%02d-%02d%02d%02d",
        local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
        local.tm_hour, local.tm_min, local.tm_sec);
    return buffer;
}

std::string FormatDateStamp(std::chrono::system_clock::time_point tp) {
    const auto local = ToLocalTime(tp);
    char buffer[16]{};
    std::snprintf(buffer, sizeof(buffer), "%04d%02d%02d",
        local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
    return buffer;
}

constexpr std::size_t kRollingStampLength = 15; // YYYYMMDD-HHMMSS
constexpr std::size_t kDailyStampLength = 8;    // YYYYMMDD

// Returns "rolling", "daily" or "" for non-backup files.
std::string ClassifyBackupName(const std::string& file_name) {
    constexpr const char* kPrefix = "launcher.v2.";
    constexpr const char* kSuffix = ".json";
    const auto prefix_length = std::strlen(kPrefix);
    const auto suffix_length = std::strlen(kSuffix);
    if (file_name.size() <= prefix_length + suffix_length) {
        return {};
    }
    if (file_name.compare(0, prefix_length, kPrefix) != 0) {
        return {};
    }
    if (file_name.compare(file_name.size() - suffix_length, suffix_length, kSuffix) != 0) {
        return {};
    }
    const auto stamp = file_name.substr(prefix_length, file_name.size() - prefix_length - suffix_length);
    if (stamp.size() == kRollingStampLength && stamp[8] == '-') {
        return "rolling";
    }
    if (stamp.size() == kDailyStampLength) {
        return "daily";
    }
    return {};
}

void PruneBackups(const std::filesystem::path& backup_dir) {
    constexpr std::size_t kKeepRolling = 5;
    constexpr std::size_t kKeepDaily = 30;

    std::vector<std::string> rolling;
    std::vector<std::string> daily;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(backup_dir, ec)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto name = entry.path().filename().string();
        const auto kind = ClassifyBackupName(name);
        if (kind == "rolling") {
            rolling.push_back(name);
        } else if (kind == "daily") {
            daily.push_back(name);
        }
    }

    // Stamp names sort lexicographically == chronologically; newest first.
    std::sort(rolling.begin(), rolling.end(), std::greater<std::string>());
    std::sort(daily.begin(), daily.end(), std::greater<std::string>());

    for (std::size_t i = kKeepRolling; i < rolling.size(); ++i) {
        std::filesystem::remove(backup_dir / rolling[i], ec);
    }
    for (std::size_t i = kKeepDaily; i < daily.size(); ++i) {
        std::filesystem::remove(backup_dir / daily[i], ec);
    }
}

std::string ToHexLower(const unsigned char* data, std::size_t size) {
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string out;
    out.reserve(size * 2);
    for (std::size_t i = 0; i < size; ++i) {
        out.push_back(kDigits[data[i] >> 4]);
        out.push_back(kDigits[data[i] & 0x0F]);
    }
    return out;
}

std::string ComputeMd5Hex(const std::string& content, std::string* error) {
    HCRYPTPROV provider = 0;
    HCRYPTHASH hash = 0;
    std::string failure;

    if (!CryptAcquireContextW(&provider, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        failure = "CryptAcquireContext failed";
    } else if (!CryptCreateHash(provider, CALG_MD5, 0, 0, &hash)) {
        failure = "CryptCreateHash failed";
    } else {
        const auto* bytes = reinterpret_cast<const BYTE*>(content.data());
        if (!CryptHashData(hash, bytes, static_cast<DWORD>(content.size()), 0)) {
            failure = "CryptHashData failed";
        }
    }

    std::string hex;
    if (failure.empty()) {
        DWORD hash_size = 0;
        DWORD value_size = sizeof(hash_size);
        unsigned char digest[16]{};
        DWORD digest_size = sizeof(digest);
        if (CryptGetHashParam(hash, HP_HASHSIZE, reinterpret_cast<BYTE*>(&hash_size), &value_size, 0) &&
            hash_size == sizeof(digest) &&
            CryptGetHashParam(hash, HP_HASHVAL, digest, &digest_size, 0)) {
            hex = ToHexLower(digest, sizeof(digest));
        } else {
            failure = "CryptGetHashParam failed";
        }
    }

    if (hash != 0) {
        CryptDestroyHash(hash);
    }
    if (provider != 0) {
        CryptReleaseContext(provider, 0);
    }

    if (!failure.empty()) {
        SetError(error, failure);
        return {};
    }
    return hex;
}

} // namespace

LauncherBackend::LauncherBackend(std::filesystem::path base_dir, std::filesystem::path legacy_root)
    : base_dir_(std::move(base_dir)),
      legacy_root_(std::move(legacy_root)),
      data_path_(base_dir_ / "launcher.v2.json"),
      settings_path_(base_dir_ / "nassistant.settings.json") {}

std::string LauncherBackend::Trim(const std::string& value) {
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch); });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    if (begin >= end) {
        return {};
    }
    return std::string(begin, end);
}

std::string LauncherBackend::ToLowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

bool LauncherBackend::IsSeparatorItem(const std::string& name, const std::string& target, const std::string& icon) {
    const auto trimmed = Trim(name);
    return (Trim(target).empty() && Trim(icon).empty()) ||
           (trimmed.rfind("----", 0) == 0 && trimmed.size() >= 8 && trimmed.substr(trimmed.size() - 4) == "----");
}

std::vector<std::string> LauncherBackend::SplitWindowsArgs(const std::string& arguments) {
    std::vector<std::string> out;
    std::string current;
    bool in_quotes = false;
    for (char ch : arguments) {
        if (ch == '"') {
            in_quotes = !in_quotes;
            continue;
        }
        if (!in_quotes && (ch == ' ' || ch == '\t')) {
            if (!current.empty()) {
                out.push_back(current);
                current.clear();
            }
            continue;
        }
        current.push_back(ch);
    }
    if (!current.empty()) {
        out.push_back(current);
    }
    return out;
}

std::string LauncherBackend::GenerateId(const std::string& prefix) {
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return prefix + "_" + std::to_string(now) + "_" + std::to_string(id_counter_++);
}

LauncherData LauncherBackend::DefaultLauncherData() const {
    LauncherData data;
    data.version = 2;
    Group group;
    group.id = const_cast<LauncherBackend*>(this)->GenerateId("group");
    group.name = "Common";
    group.order = 0;
    data.groups.push_back(group);
    return data;
}

Settings LauncherBackend::ParsePonerCfg(const std::filesystem::path& cfg_path) const {
    Settings settings;
    std::ifstream in(cfg_path);
    if (!in) {
        return settings;
    }

    std::map<std::string, std::string> values;
    std::string line;
    while (std::getline(in, line)) {
        auto trimmed = Trim(line);
        if (trimmed.empty() || trimmed[0] == '[') {
            continue;
        }
        const auto pos = trimmed.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        values[Trim(trimmed.substr(0, pos))] = Trim(trimmed.substr(pos + 1));
    }

    const auto it_hide = values.find("ExecuteHide");
    if (it_hide != values.end()) {
        auto value = ToLowerAscii(it_hide->second);
        settings.execute_hide = (value == "1" || value == "true");
    }

    auto it_current = values.find("CurrentTab");
    if (it_current == values.end()) {
        it_current = values.find("ActivatedTab");
    }
    if (it_current != values.end() && !Trim(it_current->second).empty()) {
        settings.current_group = it_current->second;
    }

    return settings;
}

bool LauncherBackend::Load(std::string* error) {
    static constexpr int kSupportedDataVersion = 2;

    std::error_code ec;
    std::filesystem::create_directories(base_dir_, ec);

    if (std::filesystem::exists(data_path_)) {
        try {
            auto raw = ReadTextFile(data_path_);
            if (!raw.empty()) {
                auto j = json::parse(raw);
                data_.version = j.value("version", kSupportedDataVersion);
                if (data_.version != kSupportedDataVersion) {
                    launcher::log::Warn("incompatible launcher.v2.json version=" + std::to_string(data_.version));
                    BackupCorruptedJson(data_path_);
                    // Remove the unusable file so the upcoming default-save does not
                    // rotate corrupted content into the backup directory.
                    std::filesystem::remove(data_path_, ec);
                    last_load_corrupted_ = true;
                    data_ = DefaultLauncherData();
                    data_.version = kSupportedDataVersion;
                    if (!SaveData(error)) {
                        return false;
                    }
                    loaded_ = true;
                    return true;
                }
                data_.groups.clear();
                if (!j.contains("groups") || !j["groups"].is_array()) {
                    data_ = DefaultLauncherData();
                } else {
                    for (const auto& jg : j["groups"]) {
                        Group g;
                        g.id = jg.value("id", GenerateId("group"));
                        g.name = jg.value("name", std::string("Common"));
                        g.order = jg.value("order", 0);
                        g.hidden = jg.value("hidden", false);
                        if (jg.contains("items") && jg["items"].is_array()) {
                            for (const auto& ji : jg["items"]) {
                                LaunchItem item;
                                item.id = ji.value("id", GenerateId("item"));
                                item.item_type = ji.value("itemType", std::string("app"));
                                item.name = ji.value("name", std::string());
                                item.target_path = ji.value("targetPath", std::string());
                                item.icon_location = ji.value("iconLocation", std::string());
                                item.arguments = ji.value("arguments", std::string());
                                item.launch_count = ji.value("launchCount", static_cast<std::uint64_t>(0));
                                item.enabled = ji.value("enabled", true);
                                g.items.push_back(std::move(item));
                            }
                        }
                        data_.groups.push_back(std::move(g));
                    }
                }
                if (data_.groups.empty()) {
                    data_ = DefaultLauncherData();
                    if (!SaveData(error)) {
                        return false;
                    }
                }
            }
        } catch (const std::exception& ex) {
            launcher::log::Error(std::string("parse launcher.v2.json failed: ") + ex.what());
            BackupCorruptedJson(data_path_);
            // Remove the unusable file so the upcoming default-save does not
            // rotate corrupted content into the backup directory.
            std::filesystem::remove(data_path_, ec);
            last_load_corrupted_ = true;
            data_ = DefaultLauncherData();
            data_.version = kSupportedDataVersion;
            if (!SaveData(error)) {
                return false;
            }
        }
    } else {
        const auto legacy_data_path = legacy_root_ / "Data.json";
        if (std::filesystem::exists(legacy_data_path)) {
            try {
                auto raw = ReadTextFile(legacy_data_path);
                auto legacy = json::parse(raw);

                data_.version = 2;
                data_.groups.clear();
                int order = 0;
                for (auto it = legacy.begin(); it != legacy.end(); ++it) {
                    Group group;
                    group.id = GenerateId("group");
                    group.name = it.key();
                    group.order = order++;

                    for (const auto& ji : it.value()) {
                        LaunchItem item;
                        item.id = GenerateId("item");
                        item.name = ji.value("Name", std::string());
                        item.target_path = ji.value("TargetPath", std::string());
                        item.icon_location = ji.value("IconLocation", std::string());
                        item.arguments = ji.value("Arguments", std::string());
                        item.launch_count = ji.value("Count", static_cast<std::uint64_t>(0));
                        item.item_type = IsSeparatorItem(item.name, item.target_path, item.icon_location) ? "separator" : "app";
                        item.enabled = true;
                        group.items.push_back(std::move(item));
                    }
                    data_.groups.push_back(std::move(group));
                }

                if (data_.groups.empty()) {
                    data_ = DefaultLauncherData();
                }

                if (!SaveData(error)) {
                    return false;
                }
            } catch (const std::exception& ex) {
                launcher::log::Error(std::string("parse legacy Data.json failed: ") + ex.what());
                SetError(error, std::string("parse legacy Data.json failed: ") + ex.what());
                return false;
            }
        } else {
            data_ = DefaultLauncherData();
            if (!SaveData(error)) {
                return false;
            }
        }
    }

    if (std::filesystem::exists(settings_path_)) {
        try {
            auto raw = ReadTextFile(settings_path_);
            if (!raw.empty()) {
                auto js = json::parse(raw);
                settings_.hotkey = js.value("hotkey", std::string("Alt+1"));
                settings_.execute_hide = js.value("executeHide", true);
                if (js.contains("currentGroup") && !js["currentGroup"].is_null()) {
                    settings_.current_group = js["currentGroup"].get<std::string>();
                }
                settings_.group_panel_width = js.value("groupPanelWidth", 220.0);
                settings_.main_window_width = js.value("mainWindowWidth", 1040.0);
                settings_.main_window_height = js.value("mainWindowHeight", 700.0);
            }
        } catch (const std::exception& ex) {
            SetError(error, std::string("parse settings failed: ") + ex.what());
            return false;
        }
    } else {
        const auto cfg = legacy_root_ / "Poner.cfg";
        if (std::filesystem::exists(cfg)) {
            settings_ = ParsePonerCfg(cfg);
        }
        if (!SaveSettings(error)) {
            return false;
        }
    }

    loaded_ = true;
    return true;
}

bool LauncherBackend::SaveData(std::string* error) const {
    RotateBackupsBeforeSave();

    json j;
    j["version"] = data_.version;
    j["groups"] = json::array();
    for (const auto& g : data_.groups) {
        json jg;
        jg["id"] = g.id;
        jg["name"] = g.name;
        jg["order"] = g.order;
        jg["hidden"] = g.hidden;
        jg["items"] = json::array();
        for (const auto& i : g.items) {
            json ji;
            ji["id"] = i.id;
            ji["itemType"] = i.item_type;
            ji["name"] = i.name;
            ji["targetPath"] = i.target_path;
            ji["iconLocation"] = i.icon_location;
            ji["arguments"] = i.arguments;
            ji["launchCount"] = i.launch_count;
            ji["enabled"] = i.enabled;
            jg["items"].push_back(std::move(ji));
        }
        j["groups"].push_back(std::move(jg));
    }

    return WriteTextAtomic(data_path_, j.dump(2), error);
}

bool LauncherBackend::SaveSettings(std::string* error) const {
    json j;
    j["hotkey"] = settings_.hotkey;
    j["executeHide"] = settings_.execute_hide;
    j["currentGroup"] = settings_.current_group.has_value() ? json(*settings_.current_group) : json(nullptr);
    j["groupPanelWidth"] = settings_.group_panel_width;
    j["mainWindowWidth"] = settings_.main_window_width;
    j["mainWindowHeight"] = settings_.main_window_height;
    return WriteTextAtomic(settings_path_, j.dump(2), error);
}

bool LauncherBackend::EnsureLoaded(std::string* error) const {
    if (!loaded_) {
        SetError(error, "backend not loaded");
        return false;
    }
    return true;
}

Group* LauncherBackend::FindGroup(const std::string& group_id) {
    auto it = std::find_if(data_.groups.begin(), data_.groups.end(), [&](const Group& g) { return g.id == group_id; });
    return it == data_.groups.end() ? nullptr : &(*it);
}

const Group* LauncherBackend::FindGroup(const std::string& group_id) const {
    auto it = std::find_if(data_.groups.begin(), data_.groups.end(), [&](const Group& g) { return g.id == group_id; });
    return it == data_.groups.end() ? nullptr : &(*it);
}

bool LauncherBackend::IsRecycleBinId(const std::string& group_id) {
    return group_id == kRecycleBinGroupId;
}

bool LauncherBackend::IsGroupHidden(const std::string& group_id) const {
    const Group* group = FindGroup(group_id);
    return group != nullptr && group->hidden;
}

Group* LauncherBackend::EnsureRecycleBin() {
    if (auto* existing = FindGroup(kRecycleBinGroupId)) {
        return existing;
    }

    int max_order = -1;
    for (const auto& g : data_.groups) {
        if (!g.hidden) {
            max_order = std::max(max_order, g.order);
        }
    }

    Group bin;
    bin.id = kRecycleBinGroupId;
    bin.name = kRecycleBinGroupName;
    bin.order = max_order + 1;
    bin.hidden = true;
    data_.groups.push_back(std::move(bin));
    launcher::log::Info("recycle bin group created");
    return &data_.groups.back();
}

void LauncherBackend::RotateBackupsBeforeSave() const {
    std::error_code ec;
    if (!std::filesystem::exists(data_path_, ec)) {
        return;
    }

    const auto backup_dir = base_dir_ / "backups";
    std::filesystem::create_directories(backup_dir, ec);

    const auto now = std::chrono::system_clock::now();

    auto copy_backup = [&](const std::string& file_name) {
        std::error_code copy_ec;
        std::filesystem::copy_file(data_path_, backup_dir / file_name,
            std::filesystem::copy_options::overwrite_existing, copy_ec);
        if (copy_ec) {
            launcher::log::Warn("backup copy failed: " + file_name + " error=" + copy_ec.message());
        }
    };

    copy_backup("launcher.v2." + FormatFileStamp(now) + ".json");
    copy_backup("launcher.v2." + FormatDateStamp(now) + ".json");

    PruneBackups(backup_dir);
}

void LauncherBackend::AppendJournal(const std::string& action, const std::string& detail) const {
    std::error_code ec;
    std::filesystem::create_directories(base_dir_, ec);

    std::ofstream out(base_dir_ / "operations.log", std::ios::app | std::ios::binary);
    if (!out) {
        launcher::log::Warn("open operations.log failed");
        return;
    }
    out << FormatJournalTimestamp(std::chrono::system_clock::now()) << " | " << action << " | " << detail << "\n";
    if (!out.good()) {
        launcher::log::Warn("write operations.log failed");
    }
}

bool LauncherBackend::UndoLastDelete(std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }
    if (!has_last_deleted_) {
        SetError(error, "no deletion to undo");
        return false;
    }

    auto* bin = FindGroup(kRecycleBinGroupId);
    if (bin == nullptr) {
        has_last_deleted_ = false;
        SetError(error, "recycle bin not found");
        return false;
    }

    auto it = std::find_if(bin->items.begin(), bin->items.end(),
        [&](const LaunchItem& item) { return item.id == last_deleted_.item.id; });
    if (it == bin->items.end()) {
        has_last_deleted_ = false;
        SetError(error, "deleted item is no longer in recycle bin");
        return false;
    }

    auto* target = FindGroup(last_deleted_.from_group_id);
    if (target == nullptr) {
        SetError(error, "original group no longer exists");
        return false;
    }

    auto restored = *it;
    bin->items.erase(it);

    const auto index = std::min(last_deleted_.index, target->items.size());
    target->items.insert(target->items.begin() + static_cast<std::ptrdiff_t>(index), std::move(restored));

    AppendJournal("undo_delete", "name=" + last_deleted_.item.name + " group=" + last_deleted_.from_group_name);
    has_last_deleted_ = false;
    return SaveData(error);
}

std::vector<BackupEntry> LauncherBackend::ListBackups() const {    std::vector<BackupEntry> out;
    const auto backup_dir = base_dir_ / "backups";
    std::error_code ec;
    if (!std::filesystem::exists(backup_dir, ec)) {
        return out;
    }

    for (const auto& entry : std::filesystem::directory_iterator(backup_dir, ec)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto name = entry.path().filename().string();
        const auto kind = ClassifyBackupName(name);
        if (kind.empty()) {
            continue;
        }
        BackupEntry item;
        item.path = entry.path();
        item.name = name;
        item.kind = kind;
        const auto mtime = std::filesystem::last_write_time(entry.path(), ec);
        if (!ec) {
            item.modified_time = std::chrono::duration_cast<std::chrono::seconds>(
                mtime.time_since_epoch()).count();
        }
        item.size = entry.file_size(ec);
        out.push_back(std::move(item));
    }

    std::sort(out.begin(), out.end(), [](const BackupEntry& lhs, const BackupEntry& rhs) {
        return lhs.name > rhs.name;
    });
    return out;
}

bool LauncherBackend::RestoreFromBackup(const std::filesystem::path& backup_path, std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }

    std::error_code ec;
    if (!std::filesystem::exists(backup_path, ec)) {
        SetError(error, "backup file not found");
        return false;
    }

    const auto content = ReadTextFile(backup_path);
    bool valid = false;
    try {
        auto j = json::parse(content);
        valid = j.value("version", 0) == 2 && j.contains("groups") && j["groups"].is_array();
    } catch (const std::exception&) {
        valid = false;
    }
    if (!valid) {
        SetError(error, "backup file is not a valid v2 dataset");
        return false;
    }

    if (!WriteTextAtomic(data_path_, content, error)) {
        return false;
    }

    AppendJournal("restore_backup", "from=" + backup_path.filename().string());
    last_load_corrupted_ = false;
    return Load(error);
}

std::string LauncherBackend::ComputeDataMd5Hex(std::string* error) const {
    if (!std::filesystem::exists(data_path_)) {
        SetError(error, "data file not found");
        return {};
    }
    const auto content = ReadTextFile(data_path_);
    return ComputeMd5Hex(content, error);
}

bool LauncherBackend::ConsumeLastLoadCorrupted() {
    return std::exchange(last_load_corrupted_, false);
}

std::string LauncherBackend::AddGroup(const std::string& name, std::string* error) {
    if (!EnsureLoaded(error)) {
        return {};
    }
    const auto group_name = Trim(name);
    if (group_name.empty()) {
        SetError(error, "group name is empty");
        return {};
    }
    const auto found = std::find_if(data_.groups.begin(), data_.groups.end(), [&](const Group& g) {
        return !g.hidden && ToLowerAscii(g.name) == ToLowerAscii(group_name);
    });
    if (found != data_.groups.end()) {
        SetError(error, "group already exists");
        return {};
    }

    int max_order = -1;
    for (const auto& g : data_.groups) {
        if (!g.hidden) {
            max_order = std::max(max_order, g.order);
        }
    }

    Group g;
    g.id = GenerateId("group");
    g.name = group_name;
    g.order = max_order + 1;
    data_.groups.push_back(g);

    AppendJournal("add_group", "id=" + g.id + " name=" + g.name);
    if (!SaveData(error)) {
        return {};
    }
    return g.id;
}

bool LauncherBackend::RenameGroup(const std::string& group_id, const std::string& name, std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }
    const auto next_name = Trim(name);
    if (next_name.empty()) {
        SetError(error, "group name is empty");
        return false;
    }

    const auto conflict = std::find_if(data_.groups.begin(), data_.groups.end(), [&](const Group& g) {
        return !g.hidden && g.id != group_id && ToLowerAscii(g.name) == ToLowerAscii(next_name);
    });
    if (conflict != data_.groups.end()) {
        SetError(error, "group already exists");
        return false;
    }

    auto* group = FindGroup(group_id);
    if (!group) {
        SetError(error, "group not found");
        return false;
    }
    if (group->hidden) {
        SetError(error, "cannot rename recycle bin");
        return false;
    }

    const auto old_name = group->name;
    group->name = next_name;
    AppendJournal("rename_group", "id=" + group_id + " from=" + old_name + " to=" + next_name);
    return SaveData(error);
}

bool LauncherBackend::DeleteGroup(const std::string& group_id, const std::string& target_group_id, std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }

    if (data_.groups.size() <= 1) {
        SetError(error, "cannot delete the last group");
        return false;
    }

    if (IsRecycleBinId(group_id)) {
        SetError(error, "cannot delete recycle bin");
        return false;
    }
    if (IsRecycleBinId(target_group_id)) {
        SetError(error, "cannot merge into recycle bin");
        return false;
    }

    auto delete_it = std::find_if(data_.groups.begin(), data_.groups.end(), [&](const Group& g) { return g.id == group_id; });
    if (delete_it == data_.groups.end()) {
        SetError(error, "group not found");
        return false;
    }

    auto target_it = std::find_if(data_.groups.begin(), data_.groups.end(), [&](const Group& g) {
        return g.id == target_group_id && g.id != group_id;
    });
    if (target_it == data_.groups.end()) {
        SetError(error, "target group not found");
        return false;
    }

    std::vector<LaunchItem> moved_items;
    moved_items.reserve(delete_it->items.size());
    for (const auto& item : delete_it->items) {
        moved_items.push_back(item);
    }
    target_it->items.insert(target_it->items.end(), moved_items.begin(), moved_items.end());
    data_.groups.erase(delete_it);

    std::vector<Group*> ordered;
    ordered.reserve(data_.groups.size());
    for (auto& group : data_.groups) {
        ordered.push_back(&group);
    }
    std::sort(ordered.begin(), ordered.end(), [](const Group* lhs, const Group* rhs) {
        return lhs->order < rhs->order;
    });
    for (int i = 0; i < static_cast<int>(ordered.size()); ++i) {
        ordered[i]->order = i;
    }

    if (settings_.current_group.has_value() && *settings_.current_group == group_id) {
        settings_.current_group = target_group_id;
    }

    AppendJournal("delete_group", "id=" + group_id + " name=" + delete_it->name + " merged_into=" + target_group_id);
    return SaveData(error);
}

bool LauncherBackend::UpsertItem(const std::string& group_id, const ItemInput& input, std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }
    auto* group = FindGroup(group_id);
    if (!group) {
        SetError(error, "group not found");
        return false;
    }
    if (group->hidden) {
        SetError(error, "cannot modify recycle bin directly");
        return false;
    }

    const auto item_type = input.item_type.has_value() ? *input.item_type :
        (IsSeparatorItem(input.name, input.target_path, input.icon_location) ? "separator" : "app");
    const auto target = Trim(input.target_path);
    const auto icon = (item_type == "app" && Trim(input.icon_location).empty()) ? target : Trim(input.icon_location);
    const auto args = Trim(input.arguments);
    const auto name = Trim(input.name);
    const auto enabled = input.enabled.value_or(true);

    if (input.id.has_value()) {
        auto it = std::find_if(group->items.begin(), group->items.end(), [&](const LaunchItem& i) { return i.id == *input.id; });
        if (it != group->items.end()) {
            it->item_type = item_type;
            it->name = name;
            it->target_path = target;
            it->icon_location = icon;
            it->arguments = args;
            it->enabled = enabled;
            AppendJournal("update_item", "id=" + it->id + " name=" + name + " group=" + group->name);
        } else {
            LaunchItem item;
            item.id = *input.id;
            item.item_type = item_type;
            item.name = name;
            item.target_path = target;
            item.icon_location = icon;
            item.arguments = args;
            item.enabled = enabled;
            AppendJournal("add_item", "id=" + item.id + " name=" + name + " group=" + group->name);
            group->items.push_back(std::move(item));
        }
    } else {
        LaunchItem item;
        item.id = GenerateId("item");
        item.item_type = item_type;
        item.name = name;
        item.target_path = target;
        item.icon_location = icon;
        item.arguments = args;
        item.enabled = enabled;
        AppendJournal("add_item", "id=" + item.id + " name=" + name + " group=" + group->name);
        group->items.push_back(std::move(item));
    }

    return SaveData(error);
}

bool LauncherBackend::DeleteItem(const std::string& group_id, const std::string& item_id, std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }

    // Deleting inside the recycle bin is the real, permanent delete.
    if (IsRecycleBinId(group_id)) {
        auto* bin = FindGroup(group_id);
        if (!bin) {
            SetError(error, "group not found");
            return false;
        }
        auto item_it = std::find_if(bin->items.begin(), bin->items.end(),
            [&](const LaunchItem& item) { return item.id == item_id; });
        if (item_it == bin->items.end()) {
            SetError(error, "item not found");
            return false;
        }
        const auto purged_name = item_it->name;
        bin->items.erase(item_it);
        if (has_last_deleted_ && last_deleted_.item.id == item_id) {
            has_last_deleted_ = false;
        }
        AppendJournal("purge_item", "id=" + item_id + " name=" + purged_name);
        return SaveData(error);
    }

    // Soft delete: move into the built-in recycle bin group.
    // EnsureRecycleBin may grow data_.groups, so (re)acquire the source group after it.
    auto* bin = EnsureRecycleBin();
    auto* group = FindGroup(group_id);
    if (!group) {
        SetError(error, "group not found");
        return false;
    }
    if (group->hidden) {
        SetError(error, "cannot modify hidden groups directly");
        return false;
    }

    auto item_it = std::find_if(group->items.begin(), group->items.end(),
        [&](const LaunchItem& item) { return item.id == item_id; });
    if (item_it == group->items.end()) {
        SetError(error, "item not found");
        return false;
    }

    DeletedItemSnapshot snapshot;
    snapshot.item = *item_it;
    snapshot.from_group_id = group->id;
    snapshot.from_group_name = group->name;
    snapshot.index = static_cast<std::size_t>(std::distance(group->items.begin(), item_it));

    group->items.erase(item_it);
    bin->items.push_back(snapshot.item);

    last_deleted_ = std::move(snapshot);
    has_last_deleted_ = true;

    AppendJournal("delete_item", "id=" + last_deleted_.item.id + " name=" + last_deleted_.item.name +
        " from=" + last_deleted_.from_group_name + " to=recycle_bin");
    return SaveData(error);
}

bool LauncherBackend::MoveItem(const std::string& group_id, const std::string& item_id, const std::string& target_group_id, std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }
    if (group_id == target_group_id) {
        SetError(error, "source and target group are the same");
        return false;
    }
    if (IsRecycleBinId(group_id)) {
        SetError(error, "cannot move out of recycle bin; use undo");
        return false;
    }
    if (IsRecycleBinId(target_group_id)) {
        SetError(error, "cannot move into recycle bin; use delete");
        return false;
    }

    auto* from = FindGroup(group_id);
    auto* to = FindGroup(target_group_id);
    if (!from) {
        SetError(error, "source group not found");
        return false;
    }
    if (!to) {
        SetError(error, "target group not found");
        return false;
    }

    auto it = std::find_if(from->items.begin(), from->items.end(), [&](const LaunchItem& item) { return item.id == item_id; });
    if (it == from->items.end()) {
        SetError(error, "item not found");
        return false;
    }

    auto moved = *it;
    from->items.erase(it);
    to->items.push_back(std::move(moved));

    AppendJournal("move_item", "id=" + item_id + " from=" + from->name + " to=" + to->name);
    return SaveData(error);
}

bool LauncherBackend::ReorderGroup(const std::string& group_id, int target_index, std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }
    if (IsRecycleBinId(group_id)) {
        SetError(error, "cannot reorder recycle bin");
        return false;
    }
    if (data_.groups.empty()) {
        SetError(error, "group list is empty");
        return false;
    }

    std::vector<Group*> ordered;
    ordered.reserve(data_.groups.size());
    for (auto& group : data_.groups) {
        if (!group.hidden) {
            ordered.push_back(&group);
        }
    }
    std::sort(ordered.begin(), ordered.end(), [](const Group* lhs, const Group* rhs) {
        return lhs->order < rhs->order;
    });

    const auto from_it = std::find_if(ordered.begin(), ordered.end(), [&](const Group* group) {
        return group->id == group_id;
    });
    if (from_it == ordered.end()) {
        SetError(error, "group not found");
        return false;
    }

    const int count = static_cast<int>(ordered.size());
    if (target_index < 0 || target_index >= count) {
        SetError(error, "target index out of range");
        return false;
    }

    const int from_index = static_cast<int>(std::distance(ordered.begin(), from_it));
    if (from_index == target_index) {
        return true;
    }

    if (from_index < target_index) {
        std::rotate(ordered.begin() + from_index, ordered.begin() + from_index + 1, ordered.begin() + target_index + 1);
    } else {
        std::rotate(ordered.begin() + target_index, ordered.begin() + from_index, ordered.begin() + from_index + 1);
    }

    for (int i = 0; i < static_cast<int>(ordered.size()); ++i) {
        ordered[i]->order = i;
    }

    AppendJournal("reorder_group", "id=" + group_id + " to_index=" + std::to_string(target_index));
    return SaveData(error);
}

bool LauncherBackend::ReorderItemInGroup(const std::string& group_id, const std::string& item_id, int target_index, std::string* error) {
    if (!EnsureLoaded(error)) {
        return false;
    }

    auto* group = FindGroup(group_id);
    if (!group) {
        SetError(error, "group not found");
        return false;
    }

    const int count = static_cast<int>(group->items.size());
    if (count == 0) {
        SetError(error, "item list is empty");
        return false;
    }
    if (target_index < 0 || target_index >= count) {
        SetError(error, "target index out of range");
        return false;
    }

    const auto from_it = std::find_if(group->items.begin(), group->items.end(), [&](const LaunchItem& item) {
        return item.id == item_id;
    });
    if (from_it == group->items.end()) {
        SetError(error, "item not found");
        return false;
    }

    const int from_index = static_cast<int>(std::distance(group->items.begin(), from_it));
    if (from_index == target_index) {
        return true;
    }

    if (from_index < target_index) {
        std::rotate(group->items.begin() + from_index, group->items.begin() + from_index + 1, group->items.begin() + target_index + 1);
    } else {
        std::rotate(group->items.begin() + target_index, group->items.begin() + from_index, group->items.begin() + from_index + 1);
    }

    AppendJournal("reorder_item", "id=" + item_id + " group=" + group->name + " to_index=" + std::to_string(target_index));
    return SaveData(error);
}

LaunchResult LauncherBackend::Launch(const std::string& group_id, const std::string& item_id, std::string* error) {
    LaunchResult result;
    if (!EnsureLoaded(error)) {
        result.message = error ? *error : "backend not loaded";
        return result;
    }

    auto* group = FindGroup(group_id);
    if (!group) {
        SetError(error, "group not found");
        result.message = "group not found";
        return result;
    }
    if (IsRecycleBinId(group_id)) {
        SetError(error, "item is in recycle bin");
        result.message = "item is in recycle bin";
        return result;
    }

    auto it = std::find_if(group->items.begin(), group->items.end(), [&](const LaunchItem& item) { return item.id == item_id; });
    if (it == group->items.end()) {
        SetError(error, "item not found");
        result.message = "item not found";
        return result;
    }

    if (it->item_type == "separator") {
        SetError(error, "separator item cannot be launched");
        result.message = "separator item cannot be launched";
        return result;
    }

    if (Trim(it->target_path).empty()) {
        SetError(error, "target path is empty");
        result.message = "target path is empty";
        return result;
    }

    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"open";

    std::wstring target_w = launcher::util::Utf8ToWide(it->target_path);
    std::wstring args_w = launcher::util::Utf8ToWide(it->arguments);
    sei.lpFile = target_w.c_str();
    sei.lpParameters = args_w.empty() ? nullptr : args_w.c_str();
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        SetError(error, "launch failed");
        result.message = "launch failed";
        return result;
    }

    if (sei.hProcess) {
        CloseHandle(sei.hProcess);
    }

    it->launch_count++;
    SaveData(nullptr);

    AppendJournal("launch", "id=" + it->id + " name=" + it->name + " count=" + std::to_string(it->launch_count));

    result.ok = true;
    result.message = "launched: " + it->name;
    return result;
}

std::string LauncherBackend::BasenameNoExt(const std::string& path) {
    std::filesystem::path p(path);
    return p.stem().string();
}

std::string LauncherBackend::PercentDecodePath(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            const auto hex = input.substr(i + 1, 2);
            char* end = nullptr;
            const auto val = static_cast<char>(std::strtol(hex.c_str(), &end, 16));
            if (end && *end == '\0') {
                out.push_back(val);
                i += 2;
                continue;
            }
        }
        out.push_back(input[i]);
    }
    return out;
}

std::string LauncherBackend::NormalizeDroppedPath(const std::string& raw_path) {
    auto value = Trim(raw_path);
    if (!value.empty() && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }

    auto lower = ToLowerAscii(value);
    if (lower.rfind("file://", 0) == 0) {
        if (lower.rfind("file:///", 0) == 0) {
            value = value.substr(8);
        } else {
            value = value.substr(7);
        }
        std::replace(value.begin(), value.end(), '/', '\\');
        value = PercentDecodePath(value);
    }

    return value;
}

std::optional<std::pair<std::string, std::string>> LauncherBackend::ResolveShortcutTarget(const std::string& shortcut_path) {
    CComPtr<IShellLinkW> shell_link;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void**>(&shell_link)))) {
        return std::nullopt;
    }

    CComPtr<IPersistFile> persist_file;
    if (FAILED(shell_link->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&persist_file)))) {
        return std::nullopt;
    }

    std::wstring link_w = launcher::util::Utf8ToWide(shortcut_path);
    if (FAILED(persist_file->Load(link_w.c_str(), STGM_READ))) {
        return std::nullopt;
    }

    wchar_t target[MAX_PATH] = {0};
    wchar_t args[2048] = {0};
    WIN32_FIND_DATAW fd{};

    if (FAILED(shell_link->GetPath(target, MAX_PATH, &fd, SLGP_RAWPATH))) {
        return std::nullopt;
    }
    shell_link->GetArguments(args, 2048);

    std::wstring target_w(target);
    if (target_w.empty()) {
        return std::nullopt;
    }

    std::wstring args_w(args);
    return std::make_pair(launcher::util::WideToUtf8(target_w), launcher::util::WideToUtf8(args_w));
}

std::size_t LauncherBackend::CreateItemsFromDroppedPaths(const std::string& group_id, const std::vector<std::string>& paths, std::string* error) {
    if (!EnsureLoaded(error)) {
        return 0;
    }

    auto* group = FindGroup(group_id);
    if (!group) {
        SetError(error, "group not found");
        return 0;
    }
    if (IsRecycleBinId(group_id)) {
        SetError(error, "cannot drop into recycle bin");
        return 0;
    }

    std::size_t created = 0;
    for (const auto& raw : paths) {
        auto dropped = NormalizeDroppedPath(raw);
        if (dropped.empty()) {
            continue;
        }

        std::string target = dropped;
        std::string args;

        auto lower = ToLowerAscii(dropped);
        if (lower.size() >= 4 && lower.substr(lower.size() - 4) == ".lnk") {
            auto resolved = ResolveShortcutTarget(dropped);
            if (!resolved.has_value()) {
                continue;
            }
            target = Trim(resolved->first);
            args = Trim(resolved->second);
            if (target.empty()) {
                continue;
            }
        }

        auto name = BasenameNoExt(target);
        if (name.empty()) {
            continue;
        }

        LaunchItem item;
        item.id = GenerateId("item");
        item.item_type = "app";
        item.name = name;
        item.target_path = target;
        item.icon_location = target;
        item.arguments = args;
        item.launch_count = 0;
        item.enabled = true;
        group->items.push_back(std::move(item));
        ++created;
    }

    if (created > 0) {
        AppendJournal("import_drop", "count=" + std::to_string(created) + " group=" + group->name);
        SaveData(nullptr);
    }
    return created;
}

} // namespace backend
