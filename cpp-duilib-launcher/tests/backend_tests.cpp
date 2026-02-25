#include "backend.h"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace {

std::filesystem::path MakeTempDir(const char* name) {
    const auto root = std::filesystem::temp_directory_path() / "nassistant_cpp_backend_tests" / name;
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root, ec);
    return root;
}

void WriteText(const std::filesystem::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << text;
}

TEST(BackendTest, ImportsLegacyDataJson) {
    const auto legacy = MakeTempDir("legacy_import");
    const auto base = MakeTempDir("base_import");

    WriteText(legacy / "Data.json", R"({
        "Common": [
            {
                "Name": "Everything",
                "TargetPath": "C:\\Program Files\\Everything\\Everything.exe",
                "IconLocation": "C:\\Program Files\\Everything\\Everything.exe",
                "Arguments": "",
                "Count": 12
            }
        ]
    })");

    backend::LauncherBackend b(base, legacy);
    std::string error;
    ASSERT_TRUE(b.Load(&error)) << error;
    ASSERT_FALSE(b.Data().groups.empty());
    EXPECT_EQ(b.Data().groups[0].name, "Common");
    ASSERT_FALSE(b.Data().groups[0].items.empty());
    EXPECT_EQ(b.Data().groups[0].items[0].name, "Everything");
    EXPECT_TRUE(std::filesystem::exists(base / "launcher.v2.json"));
}

TEST(BackendTest, GroupAndItemCrudWorks) {
    const auto legacy = MakeTempDir("legacy_crud");
    const auto base = MakeTempDir("base_crud");

    backend::LauncherBackend b(base, legacy);
    std::string error;
    ASSERT_TRUE(b.Load(&error)) << error;

    const auto id = b.AddGroup("Tools", &error);
    ASSERT_FALSE(id.empty()) << error;
    ASSERT_TRUE(b.RenameGroup(id, "MyTools", &error)) << error;

    backend::ItemInput input;
    input.name = "Procmon";
    input.target_path = "C:\\Tools\\Procmon.exe";
    input.icon_location = "C:\\Tools\\Procmon.exe";
    input.arguments = "";

    ASSERT_TRUE(b.UpsertItem(id, input, &error)) << error;

    const auto* group = [&]() -> const backend::Group* {
        for (const auto& g : b.Data().groups) {
            if (g.id == id) {
                return &g;
            }
        }
        return nullptr;
    }();

    ASSERT_NE(group, nullptr);
    ASSERT_EQ(group->name, "MyTools");
    ASSERT_EQ(group->items.size(), 1u);
    const auto item_id = group->items[0].id;

    ASSERT_TRUE(b.DeleteItem(id, item_id, &error)) << error;

    const auto* group_after = [&]() -> const backend::Group* {
        for (const auto& g : b.Data().groups) {
            if (g.id == id) {
                return &g;
            }
        }
        return nullptr;
    }();
    ASSERT_NE(group_after, nullptr);
    EXPECT_TRUE(group_after->items.empty());
}

TEST(BackendTest, MoveItemAcrossGroups) {
    const auto legacy = MakeTempDir("legacy_move");
    const auto base = MakeTempDir("base_move");

    backend::LauncherBackend b(base, legacy);
    std::string error;
    ASSERT_TRUE(b.Load(&error)) << error;

    const auto g1 = b.AddGroup("G1", &error);
    const auto g2 = b.AddGroup("G2", &error);
    ASSERT_FALSE(g1.empty());
    ASSERT_FALSE(g2.empty());

    backend::ItemInput input;
    input.name = "Notepad";
    input.target_path = "C:\\Windows\\notepad.exe";
    input.icon_location = "C:\\Windows\\notepad.exe";
    input.arguments = "";
    ASSERT_TRUE(b.UpsertItem(g1, input, &error)) << error;

    std::string item_id;
    for (const auto& g : b.Data().groups) {
        if (g.id == g1 && !g.items.empty()) {
            item_id = g.items[0].id;
        }
    }
    ASSERT_FALSE(item_id.empty());

    ASSERT_TRUE(b.MoveItem(g1, item_id, g2, &error)) << error;

    std::size_t c1 = 0;
    std::size_t c2 = 0;
    for (const auto& g : b.Data().groups) {
        if (g.id == g1) c1 = g.items.size();
        if (g.id == g2) c2 = g.items.size();
    }
    EXPECT_EQ(c1, 0u);
    EXPECT_EQ(c2, 1u);
}

TEST(BackendTest, RecoversFromIncompatibleVersion) {
    const auto legacy = MakeTempDir("legacy_incompatible_version");
    const auto base = MakeTempDir("base_incompatible_version");

    WriteText(base / "launcher.v2.json", R"({
        "version": 999,
        "groups": []
    })");

    backend::LauncherBackend b(base, legacy);
    std::string error;
    ASSERT_TRUE(b.Load(&error)) << error;
    ASSERT_EQ(b.Data().version, 2);
    ASSERT_FALSE(b.Data().groups.empty());

    bool has_backup = false;
    for (const auto& entry : std::filesystem::directory_iterator(base)) {
        const auto name = entry.path().filename().string();
        if (name.find("launcher.v2.json.bad.") == 0) {
            has_backup = true;
            break;
        }
    }
    EXPECT_TRUE(has_backup);
}

TEST(BackendTest, RecoversFromCorruptedJson) {
    const auto legacy = MakeTempDir("legacy_corrupted_json");
    const auto base = MakeTempDir("base_corrupted_json");

    WriteText(base / "launcher.v2.json", "{ invalid json");

    backend::LauncherBackend b(base, legacy);
    std::string error;
    ASSERT_TRUE(b.Load(&error)) << error;
    ASSERT_FALSE(b.Data().groups.empty());

    bool has_backup = false;
    for (const auto& entry : std::filesystem::directory_iterator(base)) {
        const auto name = entry.path().filename().string();
        if (name.find("launcher.v2.json.bad.") == 0) {
            has_backup = true;
            break;
        }
    }
    EXPECT_TRUE(has_backup);
}

} // namespace
