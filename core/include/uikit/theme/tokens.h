#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "uikit/theme/color.h"

namespace uikit {

// L1 令牌（design/*.json）的解析产物。字段固定、schema 严格校验：
// 未知键一律报错，防止令牌表漂移；这里只存数据，派生色在 resolve() 里做。
struct FontToken {
    std::string role;
    std::string family;
    int size_px = 12;
    int weight = 400;
};

struct ThemeColors {
    Color text = 0;
    Color surface = 0;
    Color surface_hover = 0;
    Color surface_selected = 0;
    Color panel = 0;
    Color panel_hover = 0;
    Color panel_selected = 0;
    Color button_hover = 0;
    Color icon_hot = 0;
    Color icon_pressed = 0;
    Color border = 0;
    Color border_strong = 0;
    Color border_focus = 0;
    Color divider = 0;
    Color row_selected_border = 0;
    Color accent = 0;
    Color on_accent = 0;
};

// 基准值一律为 96 DPI 的逻辑像素，缩放由消费方（duilib DPIObj 等）负责。
struct ThemeMetrics {
    int control_height = 28;
    int titlebar_height = 35;
    int search_height = 30;
    int icon_button_size = 30;
    int checkbox_box = 14;
    int check_stroke = 2;
    int control_hpad = 8;
    int window_inset = 12;
    int scrollbar_width = 12;
    int radius_control = 4;
    int radius_checkbox = 3;
};

struct ThemeTokens {
    color::Appearance appearance = color::Appearance::Light;
    std::string name;
    ThemeColors color;
    ThemeMetrics metrics;
    std::vector<FontToken> fonts;

    // 严格解析：schema 不符、未知键、坏色值都返回 false，err 给出定位信息。
    static bool parse(std::string_view json, ThemeTokens& out, std::string& err);
};

// icons.map.json：语义名 → micon 图标名。控件只认语义名，资产由 micon 提供。
struct IconMap {
    std::vector<std::pair<std::string, std::string>> entries;

    const std::string* find(const std::string& semantic) const;
    static bool parse(std::string_view json, IconMap& out, std::string& err);
};

}  // namespace uikit
