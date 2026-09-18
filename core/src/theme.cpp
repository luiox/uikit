#include "uikit/theme/theme.h"

#include "uikit/embedded_themes.h"

namespace uikit {

ResolvedTheme resolve(const ThemeTokens& tokens) {
    ResolvedTheme out;
    out.appearance = tokens.appearance;
    out.name = tokens.name;
    out.color = tokens.color;
    out.metrics = tokens.metrics;
    out.fonts = tokens.fonts;

    const Color text = tokens.color.text;
    const Color panel = tokens.color.panel;
    const Color surface = tokens.color.surface;
    const Color accent = tokens.color.accent;

    // 派生规则对明暗两套主题对称：以 panel 为"远离方向"，方向感由色值本身给出。
    out.text_secondary = color::mix(text, panel, 0.55);
    out.text_disabled = color::mix(text, panel, 0.75);
    out.accent_hover = color::hover_of(accent, tokens.appearance);
    out.accent_pressed = color::pressed_of(accent, tokens.appearance);
    out.control_disabled = color::mix(surface, panel, 0.6);
    out.scrollbar_thumb = color::mix(tokens.color.border_strong, panel, 0.35);
    return out;
}

namespace {

ResolvedTheme parse_embedded(const char* json) {
    ThemeTokens tokens;
    std::string err;
    if (ThemeTokens::parse(json, tokens, err)) {
        return resolve(tokens);
    }
    // 内嵌数据受单测保护，走不到这里；兜底返回一个纯结构零值。
    return ResolvedTheme{};
}

}  // namespace

const ResolvedTheme& LightTheme() {
    static const ResolvedTheme theme = parse_embedded(embed::light_json());
    return theme;
}

const ResolvedTheme& DarkTheme() {
    static const ResolvedTheme theme = parse_embedded(embed::dark_json());
    return theme;
}

const IconMap& DefaultIcons() {
    static const IconMap icons = [] {
        IconMap out;
        std::string err;
        IconMap::parse(embed::icons_json(), out, err);
        return out;
    }();
    return icons;
}

namespace {
// 活动主题默认取 LightTheme 的引用语义：未 Set 时返回内嵌浅色。
ResolvedTheme g_active;
bool g_active_set = false;
}  // namespace

const ResolvedTheme& ActiveTheme() {
    return g_active_set ? g_active : LightTheme();
}

void SetActiveTheme(const ResolvedTheme& theme) {
    g_active = theme;
    g_active_set = true;
}

}  // namespace uikit
