#include "uikit/theme/color.h"

#include <algorithm>
#include <cmath>

namespace uikit {
namespace color {
namespace {

constexpr double kChannelMax = 255.0;

std::uint8_t blend_channel(std::uint8_t a, std::uint8_t b, double t) {
    const double v = static_cast<double>(a) + (static_cast<double>(b) - static_cast<double>(a)) * t;
    return static_cast<std::uint8_t>(std::lround(std::clamp(v, 0.0, kChannelMax)));
}

bool hex_value(char c, std::uint32_t& out) {
    if (c >= '0' && c <= '9') {
        out = static_cast<std::uint32_t>(c - '0');
        return true;
    }
    if (c >= 'a' && c <= 'f') {
        out = static_cast<std::uint32_t>(c - 'a' + 10);
        return true;
    }
    if (c >= 'A' && c <= 'F') {
        out = static_cast<std::uint32_t>(c - 'A' + 10);
        return true;
    }
    return false;
}

bool hex_byte(std::string_view s, std::uint32_t& out) {
    std::uint32_t hi = 0;
    std::uint32_t lo = 0;
    if (!hex_value(s[0], hi) || !hex_value(s[1], lo)) {
        return false;
    }
    out = (hi << 4) | lo;
    return true;
}

}  // namespace

bool parse(std::string_view text, Color& out) {
    if (text.size() >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        text.remove_prefix(2);
    } else if (!text.empty() && text[0] == '#') {
        text.remove_prefix(1);
    } else {
        return false;
    }

    std::uint32_t part[4] = {0xFF, 0, 0, 0};  // 默认补不透明 alpha
    if (text.size() == 6) {
        // RRGGBB
        if (!hex_byte(text.substr(0, 2), part[1]) || !hex_byte(text.substr(2, 2), part[2]) ||
            !hex_byte(text.substr(4, 2), part[3])) {
            return false;
        }
    } else if (text.size() == 8) {
        // 8 位十六进制统一按 AARRGGBB（duilib 属性口径）
        if (!hex_byte(text.substr(0, 2), part[0]) || !hex_byte(text.substr(2, 2), part[1]) ||
            !hex_byte(text.substr(4, 2), part[2]) || !hex_byte(text.substr(6, 2), part[3])) {
            return false;
        }
    } else {
        return false;
    }
    out = rgba(part[0], part[1], part[2], part[3]);
    return true;
}

std::string to_string(Color c) {
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';
    static const char kHex[] = "0123456789ABCDEF";
    for (int i = 0; i < 4; ++i) {
        const std::uint8_t byte = static_cast<std::uint8_t>(c >> (24 - i * 8));
        buf[2 + i * 2] = kHex[byte >> 4];
        buf[3 + i * 2] = kHex[byte & 0xF];
    }
    buf[10] = '\0';
    return std::string(buf, 10);
}

Color mix(Color a, Color b, double t) {
    t = std::clamp(t, 0.0, 1.0);
    return rgba(blend_channel(alpha(a), alpha(b), t), blend_channel(red(a), red(b), t),
                blend_channel(green(a), green(b), t), blend_channel(blue(a), blue(b), t));
}

Color lighten(Color c, double t) { return mix(c, kWhite, t); }

Color darken(Color c, double t) { return mix(c, kBlack, t); }

Color hover_of(Color base, Appearance ap) {
    return ap == Appearance::Light ? darken(base, 0.08) : lighten(base, 0.08);
}

Color pressed_of(Color base, Appearance ap) {
    return ap == Appearance::Light ? darken(base, 0.16) : lighten(base, 0.16);
}

Color disabled_of(Color base, Color background) { return mix(base, background, 0.6); }

}  // namespace color
}  // namespace uikit
