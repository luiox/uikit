# 主题系统

## 令牌（L1）

`design/light.json` / `design/dark.json`，schema `uikit.tokens/1`，严格解析
（未知键报错、17 个 color 键全部必填）。字段含义：

| 键 | 用途 | light 值来源 |
|---|---|---|
| text | 正文文字 | mlaunch 主题文字色 1A1A1A |
| surface / surface_hover / surface_selected | 输入区与条目列表的底/悬停/选中 | FFFFFF / F5F5F5 / E0E0E0 |
| panel / panel_hover / panel_selected | 窗口与顶栏的底/悬停/选中 | E6E6E6 / DCDCDC / D2D2D2 |
| button_hover | 次级按钮悬停 | D5D5D5 |
| icon_hot / icon_pressed | 顶栏图标按钮悬停/按下 | D0D0D0 / C4C4C4 |
| border / border_strong / border_focus | 通用边框/交互控件常态边/交互悬停边 | D2D2D2 / B8B8B8 / 8A8A8A |
| divider | 列表分隔/描边 | CFD7E0 |
| row_selected_border | 分组行选中描边 | CDCDCD |
| accent / on_accent | 强调色（勾选、主按钮）/ 强调色上的文字 | 1A73E8 / FFFFFF |

metrics 为 96 基准逻辑像素（缩放交给消费方），font 数组按 role 区分
（default / title，weight≥600 安装为粗体）。

**深色主题**与浅色同一族系：浅色面板灰（E6E6E6）翻转为深色文字色，
表面/面板逐级压暗；accent 取同族提亮的 8AB4F8，对勾用深色 on_accent。

## 派生（L2）

基础色一律来自令牌（浅色全部反向提炼自 mlaunch 已验证配置，未发明新值），
状态色由 `resolve()` 计算，方向随明暗主题自动翻转：

| 派生色 | 规则 |
|---|---|
| text_secondary / text_disabled | text 向 panel 混 55% / 75% |
| accent_hover / accent_pressed | accent 按明暗方向偏 8% / 16%（浅加深、深提亮） |
| control_disabled | surface 向 panel 混 60% |
| scrollbar_thumb | border_strong 向 panel 收 35% |

## 内嵌与换肤

默认主题编译期内嵌（`tools/gen_embed.sh` 生成
`core/include/uikit/embedded_themes.h`，改 `design/` 后必须重新生成，
CI 用 `git diff --exit-code` 校验同步），运行期零文件依赖。
运行期换肤/覆盖：解析外部 json → `resolve()` → `SetActiveTheme()` →
`ApplyThemeToTree(root)`。

## 切主题的正确姿势

uikit 自绘控件每帧直读 `ActiveTheme()`；构造期上色的部分（Label 文本色、
容器 bkcolor、列表条目色）由 `ApplyThemeToTree` 按类重涂（`ThemeableControl::
OnThemeChanged` / 原生 CLabelUI 统一正文色）。滚动条轨道色是存储值，
消费方需在切换后重调 `ApplyFlatScrollbar`。
