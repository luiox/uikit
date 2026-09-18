#include "uikit/theme/tokens.h"

#include <nlohmann/json.hpp>

namespace uikit {
namespace {

namespace nj = nlohmann;

const char* kSchemaTokens = "uikit.tokens/1";
const char* kSchemaIcons = "uikit.icons/1";

// color 节的 17 个键全部必填（缺一个主题就不完整），未知键报错。
const char* const kColorKeys[] = {
    "text",    "surface",  "surface_hover", "surface_selected", "panel",
    "panel_hover", "panel_selected", "button_hover",  "icon_hot",  "icon_pressed",
    "border",  "border_strong", "border_focus",  "divider", "row_selected_border",
    "accent",  "on_accent",
};

const char* const kMetricKeys[] = {
    "control_height", "titlebar_height", "search_height",   "icon_button_size",
    "checkbox_box",   "check_stroke",    "control_hpad",    "window_inset",
    "scrollbar_width", "radius_control", "radius_checkbox",
};

bool parse_color(const nj::json& j, Color& out, std::string& err, const char* where) {
    if (!j.is_string()) {
        err = std::string(where) + ": 颜色必须是字符串";
        return false;
    }
    if (!color::parse(j.get<std::string>(), out)) {
        err = std::string(where) + ": 无法解析颜色 " + j.get<std::string>();
        return false;
    }
    return true;
}

void reject_unknown(const nj::json& obj, const char* const* known, size_t count,
                    std::string& err, const char* where) {
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        bool ok = false;
        for (size_t i = 0; i < count; ++i) {
            if (it.key() == known[i]) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            err = std::string(where) + ": 未知键 \"" + it.key() + "\"";
        }
    }
}

}  // namespace

bool ThemeTokens::parse(std::string_view json, ThemeTokens& out, std::string& err) {
    const nj::json root = nj::json::parse(json, nullptr, false, /*ignore_comments*/ true);
    if (root.is_discarded() || !root.is_object()) {
        err = "顶层必须是 JSON 对象";
        return false;
    }

    // —— schema ——
    const auto schema_it = root.find("schema");
    if (schema_it == root.end() || !schema_it->is_string() ||
        schema_it->get<std::string>() != kSchemaTokens) {
        err = "schema 必须是 \"" + std::string(kSchemaTokens) + "\"";
        return false;
    }

    // —— meta ——
    const auto meta_it = root.find("meta");
    if (meta_it == root.end() || !meta_it->is_object()) {
        err = "缺少 meta 对象";
        return false;
    }
    const auto name_it = meta_it->find("name");
    if (name_it == meta_it->end() || !name_it->is_string()) {
        err = "meta.name 必须是字符串";
        return false;
    }
    out.name = name_it->get<std::string>();
    const auto appearance_it = meta_it->find("appearance");
    if (appearance_it == meta_it->end() || !appearance_it->is_string()) {
        err = "meta.appearance 必须是字符串";
        return false;
    }
    const std::string appearance = appearance_it->get<std::string>();
    if (appearance == "light") {
        out.appearance = color::Appearance::Light;
    } else if (appearance == "dark") {
        out.appearance = color::Appearance::Dark;
    } else {
        err = "meta.appearance 只认 light/dark，得到 \"" + appearance + "\"";
        return false;
    }
    for (auto it = meta_it->begin(); it != meta_it->end(); ++it) {
        if (it.key() != "name" && it.key() != "appearance" && it.key() != "description") {
            err = "meta: 未知键 \"" + it.key() + "\"";
            return false;
        }
    }

    // —— color（全键必填）——
    const auto color_it = root.find("color");
    if (color_it == root.end() || !color_it->is_object()) {
        err = "缺少 color 对象";
        return false;
    }
    Color* const color_slots[] = {
        &out.color.text,        &out.color.surface,   &out.color.surface_hover,
        &out.color.surface_selected, &out.color.panel,    &out.color.panel_hover,
        &out.color.panel_selected,   &out.color.button_hover, &out.color.icon_hot,
        &out.color.icon_pressed,     &out.color.border,   &out.color.border_strong,
        &out.color.border_focus,     &out.color.divider,  &out.color.row_selected_border,
        &out.color.accent,      &out.color.on_accent,
    };
    static_assert(sizeof(color_slots) / sizeof(color_slots[0]) == sizeof(kColorKeys) / sizeof(kColorKeys[0]));
    for (size_t i = 0; i < sizeof(kColorKeys) / sizeof(kColorKeys[0]); ++i) {
        const auto it = color_it->find(kColorKeys[i]);
        if (it == color_it->end()) {
            err = std::string("color.") + kColorKeys[i] + " 缺失";
            return false;
        }
        if (!parse_color(*it, *color_slots[i], err, (std::string("color.") + kColorKeys[i]).c_str())) {
            return false;
        }
    }
    reject_unknown(*color_it, kColorKeys, sizeof(kColorKeys) / sizeof(kColorKeys[0]), err,
                   "color");
    if (!err.empty()) {
        return false;
    }

    // —— metrics（可选，未知键报错）——
    if (const auto metrics_it = root.find("metrics"); metrics_it != root.end()) {
        if (!metrics_it->is_object()) {
            err = "metrics 必须是对象";
            return false;
        }
        int* const slots[] = {
            &out.metrics.control_height, &out.metrics.titlebar_height,
            &out.metrics.search_height,  &out.metrics.icon_button_size,
            &out.metrics.checkbox_box,   &out.metrics.check_stroke,
            &out.metrics.control_hpad,   &out.metrics.window_inset,
            &out.metrics.scrollbar_width, &out.metrics.radius_control,
            &out.metrics.radius_checkbox,
        };
        static_assert(sizeof(slots) / sizeof(slots[0]) == sizeof(kMetricKeys) / sizeof(kMetricKeys[0]));
        for (size_t i = 0; i < sizeof(kMetricKeys) / sizeof(kMetricKeys[0]); ++i) {
            const auto it = metrics_it->find(kMetricKeys[i]);
            if (it == metrics_it->end()) {
                continue;
            }
            if (!it->is_number_integer() || it->get<int>() < 0) {
                err = std::string("metrics.") + kMetricKeys[i] + " 必须是非负整数";
                return false;
            }
            *slots[i] = it->get<int>();
        }
        reject_unknown(*metrics_it, kMetricKeys, sizeof(kMetricKeys) / sizeof(kMetricKeys[0]),
                       err, "metrics");
        if (!err.empty()) {
            return false;
        }
    }

    // —— font（可选数组）——
    if (const auto fonts_it = root.find("font"); fonts_it != root.end() && !fonts_it->is_null()) {
        if (!fonts_it->is_array()) {
            err = "font 必须是数组";
            return false;
        }
        for (const auto& f : *fonts_it) {
            if (!f.is_object()) {
                err = "font 条目必须是对象";
                return false;
            }
            FontToken token;
            const auto role_it = f.find("role");
            const auto family_it = f.find("family");
            if (role_it == f.end() || !role_it->is_string() || family_it == f.end() ||
                !family_it->is_string()) {
                err = "font 条目需要字符串 role/family";
                return false;
            }
            token.role = role_it->get<std::string>();
            token.family = family_it->get<std::string>();
            if (const auto size_it = f.find("size_px"); size_it != f.end()) {
                if (!size_it->is_number_integer() || size_it->get<int>() <= 0) {
                    err = "font.size_px 必须是正整数";
                    return false;
                }
                token.size_px = size_it->get<int>();
            }
            if (const auto weight_it = f.find("weight"); weight_it != f.end()) {
                if (!weight_it->is_number_integer() || weight_it->get<int>() <= 0) {
                    err = "font.weight 必须是正整数";
                    return false;
                }
                token.weight = weight_it->get<int>();
            }
            for (auto it = f.begin(); it != f.end(); ++it) {
                if (it.key() != "role" && it.key() != "family" && it.key() != "size_px" &&
                    it.key() != "weight") {
                    err = "font 条目: 未知键 \"" + it.key() + "\"";
                    return false;
                }
            }
            out.fonts.push_back(std::move(token));
        }
    }

    // —— 未知顶层键 ——
    for (auto it = root.begin(); it != root.end(); ++it) {
        if (it.key() != "schema" && it.key() != "meta" && it.key() != "color" &&
            it.key() != "metrics" && it.key() != "font") {
            err = "未知顶层键 \"" + it.key() + "\"";
            return false;
        }
    }
    err.clear();
    return true;
}

const std::string* IconMap::find(const std::string& semantic) const {
    for (const auto& e : entries) {
        if (e.first == semantic) {
            return &e.second;
        }
    }
    return nullptr;
}

bool IconMap::parse(std::string_view json, IconMap& out, std::string& err) {
    const nj::json root = nj::json::parse(json, nullptr, false, true);
    if (root.is_discarded() || !root.is_object()) {
        err = "顶层必须是 JSON 对象";
        return false;
    }
    const auto schema_it = root.find("schema");
    if (schema_it == root.end() || !schema_it->is_string() ||
        schema_it->get<std::string>() != kSchemaIcons) {
        err = "schema 必须是 \"" + std::string(kSchemaIcons) + "\"";
        return false;
    }
    const auto map_it = root.find("map");
    if (map_it == root.end() || !map_it->is_object()) {
        err = "缺少 map 对象";
        return false;
    }
    for (auto it = map_it->begin(); it != map_it->end(); ++it) {
        if (!it->is_string()) {
            err = "map." + it.key() + " 必须是字符串（micon 图标名）";
            return false;
        }
        out.entries.emplace_back(it.key(), it->get<std::string>());
    }
    err.clear();
    return true;
}

}  // namespace uikit
