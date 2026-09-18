#include <gtest/gtest.h>

#include "uikit/theme/theme.h"
#include "uikit/theme/tokens.h"

using namespace uikit;

namespace {

// 与 design/light.json 对齐的最小合法令牌表（改字段做负例）
constexpr const char* kValidTokens = R"tk({
  "schema": "uikit.tokens/1",
  "meta": { "name": "test", "appearance": "light" },
  "color": {
    "text": "#FF1A1A1A", "surface": "#FFFFFFFF", "surface_hover": "#FFF5F5F5",
    "surface_selected": "#FFE0E0E0", "panel": "#FFE6E6E6", "panel_hover": "#FFDCDCDC",
    "panel_selected": "#FFD2D2D2", "button_hover": "#FFD5D5D5", "icon_hot": "#FFD0D0D0",
    "icon_pressed": "#FFC4C4C4", "border": "#FFD2D2D2", "border_strong": "#FFB8B8B8",
    "border_focus": "#FF8A8A8A", "divider": "#FFCFD7E0", "row_selected_border": "#FFCDCDCD",
    "accent": "#FF1A73E8", "on_accent": "#FFFFFFFF"
  },
  "metrics": { "control_height": 30 }
})tk";

}  // namespace

TEST(Tokens, EmbeddedLightParses) {
    ThemeTokens tokens;
    std::string err;
    ASSERT_TRUE(ThemeTokens::parse(embed::light_json(), tokens, err)) << err;
    EXPECT_EQ(tokens.name, "light");
    EXPECT_EQ(tokens.appearance, color::Appearance::Light);
    EXPECT_EQ(tokens.color.text, 0xFF1A1A1Au);
    EXPECT_EQ(tokens.color.accent, 0xFF1A73E8u);
    EXPECT_EQ(tokens.metrics.radius_control, 4);
    EXPECT_EQ(tokens.metrics.control_height, 28);
    ASSERT_FALSE(tokens.fonts.empty());
    EXPECT_EQ(tokens.fonts[0].role, "default");
}

TEST(Tokens, EmbeddedDarkParses) {
    ThemeTokens tokens;
    std::string err;
    ASSERT_TRUE(ThemeTokens::parse(embed::dark_json(), tokens, err)) << err;
    EXPECT_EQ(tokens.name, "dark");
    EXPECT_EQ(tokens.appearance, color::Appearance::Dark);
    EXPECT_EQ(tokens.color.accent, 0xFF8AB4F8u);
    EXPECT_EQ(tokens.color.on_accent, 0xFF1F1F1Fu);
}

TEST(Tokens, EmbeddedIconsParse) {
    IconMap icons;
    std::string err;
    ASSERT_TRUE(IconMap::parse(embed::icons_json(), icons, err)) << err;
    const std::string* icon = icons.find("window.close");
    ASSERT_NE(icon, nullptr);
    EXPECT_EQ(*icon, "close");
    EXPECT_EQ(icons.find("no.such.semantic"), nullptr);
}

TEST(Tokens, MissingColorKeyIsRejected) {
    std::string json(kValidTokens);
    const auto pos = json.find("\"divider\": \"#FFCFD7E0\",");
    ASSERT_NE(pos, std::string::npos);
    ThemeTokens tokens;
    std::string err;
    EXPECT_FALSE(ThemeTokens::parse(json.erase(pos, 23), tokens, err));
    EXPECT_NE(err.find("divider"), std::string::npos);
}

TEST(Tokens, UnknownKeyIsRejected) {
    ThemeTokens tokens;
    std::string err;
    std::string json(kValidTokens);
    json = json.replace(json.find("\"metrics\""), 9, "\"metric\"");
    EXPECT_FALSE(ThemeTokens::parse(json, tokens, err));
    EXPECT_NE(err.find("metric"), std::string::npos);
}

TEST(Tokens, BadSchemaIsRejected) {
    std::string json(kValidTokens);
    json = json.replace(json.find("uikit.tokens/1"), 14, "uikit.tokens/2");
    ThemeTokens tokens;
    std::string err;
    EXPECT_FALSE(ThemeTokens::parse(json, tokens, err));
}

TEST(Tokens, BadColorValueIsRejected) {
    std::string json(kValidTokens);
    json = json.replace(json.find("#FF1A1A1A"), 9, "yellow");
    ThemeTokens tokens;
    std::string err;
    EXPECT_FALSE(ThemeTokens::parse(json, tokens, err));
}

TEST(Tokens, MetricsPartialOverrideKeepsDefaults) {
    ThemeTokens tokens;
    std::string err;
    ASSERT_TRUE(ThemeTokens::parse(kValidTokens, tokens, err)) << err;
    EXPECT_EQ(tokens.metrics.control_height, 30);  // 覆盖
    EXPECT_EQ(tokens.metrics.checkbox_box, 14);    // 缺省
}
