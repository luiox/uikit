#pragma once

// 主题应用器：把"活动主题"落到 duilib 控件树，并提供运行期换肤入口。
// uikit 自有控件绘制时直读 ActiveTheme()，无需逐个 Set；本文件负责
// 两件事——切换后的整体失效重绘、字体安装（duilib 字体按 manager 索引管理）。

#include <UIlib.h>

#include "uikit/theme/theme.h"

namespace uikit::duilib {

// 切换活动主题后调用：递归整棵控件树——
// · uikit 控件（ThemeableControl）→ 调 OnThemeChanged 重涂构造期上色项；
// · 原生 CLabelUI → 文本色统一设为正文色（要弱化文本请用 uikit::duilib::LabelUI）；
// · 其余控件仅 Invalidate（自绘类控件下一帧直读 ActiveTheme()）。
void ApplyThemeToTree(DuiLib::CControlUI* root);

// 按 theme.fonts 安装字体：role=="default" 进 SetDefaultFont，其余 AddFont
// 并把索引记入 *out_extra_font（按 fonts 顺序，起始索引 1）。返回 default 字体索引（恒 0）。
// size_px 由调用方给（duilib 字体尺寸的 DPI 语义各 fork 有差异，不替消费方做主）。
int InstallFonts(DuiLib::CPaintManagerUI& pm, const ResolvedTheme& theme, int size_px,
                 int* out_extra_count = nullptr);

}  // namespace uikit::duilib
