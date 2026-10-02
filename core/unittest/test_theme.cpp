#include <gtest/gtest.h>

#include "uikit/theme/theme.h"

using namespace uikit;

TEST(Theme, EmbeddedLightResolves) {
    const ResolvedTheme& theme = LightTheme();
    EXPECT_EQ(theme.name, "light");
    EXPECT_EQ(theme.appearance, color::Appearance::Light);
    // 基础色直通
    EXPECT_EQ(theme.color.panel, 0xFFE6E6E6u);
    // 派生色全部就位且与基础色不同
    EXPECT_NE(theme.text_secondary, 0u);
    EXPECT_NE(theme.text_disabled, 0u);
    EXPECT_NE(theme.accent_hover, theme.color.accent);
    EXPECT_NE(theme.accent_pressed, theme.accent_hover);
    EXPECT_NE(theme.control_disabled, theme.color.surface);
    EXPECT_NE(theme.scrollbar_thumb, 0u);
    // 浅色主题下弱化文本应比正文"浅"（蓝通道更高）
    EXPECT_GT(color::blue(theme.text_secondary), color::blue(theme.color.text));
}

TEST(Theme, EmbeddedDarkResolves) {
    const ResolvedTheme& theme = DarkTheme();
    EXPECT_EQ(theme.name, "dark");
    EXPECT_EQ(theme.appearance, color::Appearance::Dark);
    // 深色面板比深色文字更暗（蓝通道更低）
    EXPECT_LT(color::blue(theme.color.panel), color::blue(theme.color.text));
    // 深色主题派生方向：accent 提亮
    EXPECT_GT(color::blue(theme.accent_hover), color::blue(theme.color.accent));
}

TEST(Theme, ActiveThemeDefaultsToLightAndSwitches) {
    EXPECT_EQ(ActiveTheme().name, "light");
    SetActiveTheme(DarkTheme());
    EXPECT_EQ(ActiveTheme().name, "dark");
    // 还原，避免影响其它用例对默认态的假设
    SetActiveTheme(LightTheme());
    EXPECT_EQ(ActiveTheme().name, "light");
}
