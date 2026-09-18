#pragma once
// 由 tools/gen_embed.sh 从 design/*.json 生成，勿手改；改 design/ 后重新生成。
namespace uikit::embed {
inline const char* light_json() { return R"tk({
  "schema": "uikit.tokens/1",
  "meta": {
    "name": "light",
    "appearance": "light",
    "description": "浅色主题。全部色值反向提炼自 mlaunch src/ui（VB6 原版逐项采样的已验证配置），未发明新值。"
  },
  "color": {
    "text": "#FF1A1A1A",
    "surface": "#FFFFFFFF",
    "surface_hover": "#FFF5F5F5",
    "surface_selected": "#FFE0E0E0",
    "panel": "#FFE6E6E6",
    "panel_hover": "#FFDCDCDC",
    "panel_selected": "#FFD2D2D2",
    "button_hover": "#FFD5D5D5",
    "icon_hot": "#FFD0D0D0",
    "icon_pressed": "#FFC4C4C4",
    "border": "#FFD2D2D2",
    "border_strong": "#FFB8B8B8",
    "border_focus": "#FF8A8A8A",
    "divider": "#FFCFD7E0",
    "row_selected_border": "#FFCDCDCD",
    "accent": "#FF1A73E8",
    "on_accent": "#FFFFFFFF"
  },
  "font": [
    { "role": "default", "family": "Microsoft YaHei UI", "size_px": 12, "weight": 400 },
    { "role": "title", "family": "Microsoft YaHei UI", "size_px": 12, "weight": 600 }
  ],
  "metrics": {
    "control_height": 28,
    "titlebar_height": 35,
    "search_height": 30,
    "icon_button_size": 30,
    "checkbox_box": 14,
    "check_stroke": 2,
    "control_hpad": 8,
    "window_inset": 12,
    "scrollbar_width": 12,
    "radius_control": 4,
    "radius_checkbox": 3
  }
}
)tk"; }
inline const char* dark_json() { return R"tk({
  "schema": "uikit.tokens/1",
  "meta": {
    "name": "dark",
    "appearance": "dark",
    "description": "深色主题。与浅色同一灰阶族系做明度翻转：浅色的面板灰（E6E6E6）在深色下转为文字色，表面/面板逐级压暗；强调蓝取同族提亮的 8AB4F8（对勾改深色）。"
  },
  "color": {
    "text": "#FFE6E6E6",
    "surface": "#FF2D2D2D",
    "surface_hover": "#FF383838",
    "surface_selected": "#FF454545",
    "panel": "#FF252525",
    "panel_hover": "#FF333333",
    "panel_selected": "#FF3D3D3D",
    "button_hover": "#FF383838",
    "icon_hot": "#FF404040",
    "icon_pressed": "#FF4A4A4A",
    "border": "#FF3D3D3D",
    "border_strong": "#FF5F5F5F",
    "border_focus": "#FF8A8A8A",
    "divider": "#FF3A424D",
    "row_selected_border": "#FF5F5F5F",
    "accent": "#FF8AB4F8",
    "on_accent": "#FF1F1F1F"
  },
  "font": [
    { "role": "default", "family": "Microsoft YaHei UI", "size_px": 12, "weight": 400 },
    { "role": "title", "family": "Microsoft YaHei UI", "size_px": 12, "weight": 600 }
  ],
  "metrics": {
    "control_height": 28,
    "titlebar_height": 35,
    "search_height": 30,
    "icon_button_size": 30,
    "checkbox_box": 14,
    "check_stroke": 2,
    "control_hpad": 8,
    "window_inset": 12,
    "scrollbar_width": 12,
    "radius_control": 4,
    "radius_checkbox": 3
  }
}
)tk"; }
inline const char* icons_json() { return R"tk({
  "schema": "uikit.icons/1",
  "description": "语义名 → micon 图标名的映射。uikit 控件只认语义名，具体图标资产由 micon 提供；运行期由消费方按 micon 的图标解析规则取 SVG。",
  "map": {
    "window.close": "close",
    "window.minimize": "minimize",
    "window.maximize": "maximize",
    "window.restore": "restore",
    "action.search": "search",
    "action.settings": "settings",
    "action.launch": "play",
    "action.open": "folder-open",
    "action.refresh": "refresh"
  }
}
)tk"; }
}  // namespace uikit::embed
