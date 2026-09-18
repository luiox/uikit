#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace uikit {

// 颜色统一为 0xAARRGGBB 的 uint32，与 duilib 的 DWORD/COLORREF 属性口径直通。
using Color = std::uint32_t;

namespace color {

inline constexpr Color kTransparent = 0x00000000;
inline constexpr Color kWhite = 0xFFFFFFFF;
inline constexpr Color kBlack = 0xFF000000;

enum class Appearance { Light, Dark };

constexpr std::uint8_t alpha(Color c) { return static_cast<std::uint8_t>(c >> 24); }
constexpr std::uint8_t red(Color c) { return static_cast<std::uint8_t>((c >> 16) & 0xFF); }
constexpr std::uint8_t green(Color c) { return static_cast<std::uint8_t>((c >> 8) & 0xFF); }
constexpr std::uint8_t blue(Color c) { return static_cast<std::uint8_t>(c & 0xFF); }

constexpr Color rgba(std::uint32_t a, std::uint32_t r, std::uint32_t g, std::uint32_t b) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// 解析 "#RRGGBB" / "#RRGGBBAA" / "#AARRGGBB" / "0xAARRGGBB"。
// 6 位十六进制补不透明 alpha；解析失败返回 false 且不动 out。
bool parse(std::string_view text, Color& out);

// 输出 "0xAARRGGBB"（duilib 属性可直接使用）。
std::string to_string(Color c);

// 线性混合，t=0 取 a，t=1 取 b，alpha 同步混合。
Color mix(Color a, Color b, double t);
Color lighten(Color c, double t);  // 向白混合
Color darken(Color c, double t);   // 向黑混合

// 状态色派生：方向由明暗决定——浅色主题加深、深色主题提亮（同一手法在两套主题下都成立）。
Color hover_of(Color base, Appearance ap);     // 偏移 8%
Color pressed_of(Color base, Appearance ap);   // 偏移 16%
// 禁用态：向背景混 60%（浅色变浅灰、深色变深灰，只靠 base 自身无法表达）。
Color disabled_of(Color base, Color background);

}  // namespace color
}  // namespace uikit
