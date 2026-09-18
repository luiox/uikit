#include <gtest/gtest.h>

#include "uikit/theme/color.h"

namespace c = uikit::color;
using uikit::Color;

TEST(Color, ParseSixDigitHexFillsOpaqueAlpha) {
    Color out = 0;
    ASSERT_TRUE(c::parse("#1A1A1A", out));
    EXPECT_EQ(out, 0xFF1A1A1Au);
}

TEST(Color, ParseEightDigitHexIsAarrggbb) {
    Color out = 0;
    ASSERT_TRUE(c::parse("#FF1A73E8", out));
    EXPECT_EQ(out, 0xFF1A73E8u);
    ASSERT_TRUE(c::parse("0x80102030", out));
    EXPECT_EQ(out, 0x80102030u);
}

TEST(Color, ParseRejectsGarbage) {
    Color out = 0x12345678;
    EXPECT_FALSE(c::parse("", out));
    EXPECT_FALSE(c::parse("#", out));
    EXPECT_FALSE(c::parse("#12345", out));
    EXPECT_FALSE(c::parse("#1234567", out));
    EXPECT_FALSE(c::parse("yellow", out));
    EXPECT_FALSE(c::parse("#GGGGGG", out));
    // 失败不动 out
    EXPECT_EQ(out, 0x12345678u);
}

TEST(Color, ToStringMatchesDuilibAttrForm) {
    EXPECT_EQ(c::to_string(0xFF1A73E8u), "0xFF1A73E8");
    EXPECT_EQ(c::to_string(0x00000000u), "0x00000000");
}

TEST(Color, MixEndpointsAndMidpoint) {
    EXPECT_EQ(c::mix(c::kBlack, c::kWhite, 0.0), c::kBlack);
    EXPECT_EQ(c::mix(c::kBlack, c::kWhite, 1.0), c::kWhite);
    EXPECT_EQ(c::mix(c::kBlack, c::kWhite, 0.5), 0xFF808080u);
    // alpha 同步混合
    EXPECT_EQ(c::alpha(c::mix(0x00112233u, 0xFFAABBCCu, 0.5)), 0x80);
}

TEST(Color, LightenDarkenExtremes) {
    EXPECT_EQ(c::darken(c::kWhite, 1.0), c::kBlack);
    EXPECT_EQ(c::lighten(c::kBlack, 1.0), c::kWhite);
}

TEST(Color, HoverPressedFollowAppearanceDirection) {
    const Color light_base = 0xFFE6E6E6;
    // 浅色主题：状态色加深
    EXPECT_LT(c::blue(c::hover_of(light_base, c::Appearance::Light)),
              c::blue(light_base));
    EXPECT_LT(c::blue(c::pressed_of(light_base, c::Appearance::Light)),
              c::blue(c::hover_of(light_base, c::Appearance::Light)));
    // 深色主题：状态色提亮
    const Color dark_base = 0xFF252525;
    EXPECT_GT(c::red(c::hover_of(dark_base, c::Appearance::Dark)), c::red(dark_base));
    EXPECT_GT(c::red(c::pressed_of(dark_base, c::Appearance::Dark)),
              c::red(c::hover_of(dark_base, c::Appearance::Dark)));
}

TEST(Color, DisabledBlendsTowardBackground) {
    // 黑向白混 60% → 153 = 0x99
    EXPECT_EQ(c::disabled_of(c::kBlack, c::kWhite), 0xFF999999u);
}
