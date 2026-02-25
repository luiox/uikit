#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace backend {

struct LaunchItem {
    std::string id;
    std::string item_type;
    std::string name;
    std::string target_path;
    std::string icon_location;
    std::string arguments;
    std::uint64_t launch_count = 0;
    bool enabled = true;
};

struct Group {
    std::string id;
    std::string name;
    int order = 0;
    std::vector<LaunchItem> items;
};

struct LauncherData {
    int version = 2;
    std::vector<Group> groups;
};

struct Settings {
    std::string hotkey = "Alt+1";
    bool execute_hide = true;
    std::optional<std::string> current_group;
    double group_panel_width = 220.0;
    double main_window_width = 1040.0;
    double main_window_height = 700.0;
};

struct ItemInput {
    std::optional<std::string> id;
    std::optional<std::string> item_type;
    std::string name;
    std::string target_path;
    std::string icon_location;
    std::string arguments;
    std::optional<bool> enabled;
};

struct LaunchResult {
    bool ok = false;
    std::string message;
};

class LauncherBackend {
public:
    LauncherBackend(std::filesystem::path base_dir, std::filesystem::path legacy_root);

    bool Load(std::string* error = nullptr);
    bool SaveData(std::string* error = nullptr) const;
    bool SaveSettings(std::string* error = nullptr) const;

    std::string AddGroup(const std::string& name, std::string* error = nullptr);
    bool RenameGroup(const std::string& group_id, const std::string& name, std::string* error = nullptr);
    bool DeleteGroup(const std::string& group_id, const std::string& target_group_id, std::string* error = nullptr);
    bool UpsertItem(const std::string& group_id, const ItemInput& input, std::string* error = nullptr);
    bool DeleteItem(const std::string& group_id, const std::string& item_id, std::string* error = nullptr);
    bool MoveItem(const std::string& group_id, const std::string& item_id, const std::string& target_group_id, std::string* error = nullptr);
    bool ReorderGroup(const std::string& group_id, int target_index, std::string* error = nullptr);
    bool ReorderItemInGroup(const std::string& group_id, const std::string& item_id, int target_index, std::string* error = nullptr);
    LaunchResult Launch(const std::string& group_id, const std::string& item_id, std::string* error = nullptr);
    std::size_t CreateItemsFromDroppedPaths(const std::string& group_id, const std::vector<std::string>& paths, std::string* error = nullptr);

    const LauncherData& Data() const { return data_; }
    const Settings& CurrentSettings() const { return settings_; }
    const std::filesystem::path& DataPath() const { return data_path_; }

private:
    static std::string Trim(const std::string& value);
    static std::string ToLowerAscii(std::string value);
    static bool IsSeparatorItem(const std::string& name, const std::string& target, const std::string& icon);
    static std::vector<std::string> SplitWindowsArgs(const std::string& arguments);
    static std::string BasenameNoExt(const std::string& path);
    static std::string NormalizeDroppedPath(const std::string& raw_path);
    static std::string PercentDecodePath(const std::string& input);
    static std::optional<std::pair<std::string, std::string>> ResolveShortcutTarget(const std::string& shortcut_path);

    std::string GenerateId(const std::string& prefix);
    LauncherData DefaultLauncherData() const;
    Settings ParsePonerCfg(const std::filesystem::path& cfg_path) const;
    bool EnsureLoaded(std::string* error) const;
    Group* FindGroup(const std::string& group_id);
    const Group* FindGroup(const std::string& group_id) const;

    std::filesystem::path base_dir_;
    std::filesystem::path legacy_root_;
    std::filesystem::path data_path_;
    std::filesystem::path settings_path_;

    LauncherData data_;
    Settings settings_;
    bool loaded_ = false;

    std::uint64_t id_counter_ = 1;
};

} // namespace backend
