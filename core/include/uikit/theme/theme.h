#pragma once

#include <string>
#include <vector>

#include "uikit/theme/tokens.h"

namespace uikit {

// L2 输出：已解析主题对象。基础色原样来自令牌；派生色由 resolve() 按明暗方向计算。
// duilib 层的控件绘制只读本结构，不接触 json。
struct ResolvedTheme {
    color::Appearance appearance = color::Appearance::Light;
    std::string name;
    ThemeColors color;
    ThemeMetrics metrics;
    std::vector<FontToken> fonts;

    // —— 以下全部为派生色（resolve() 填充）——
    Color text_secondary = 0;    // 弱化文本：text 向 panel 混 55%
    Color text_disabled = 0;     // 禁用文本：混 75%
    Color accent_hover = 0;      // 强调色悬停（按明暗方向偏 8%）
    Color accent_pressed = 0;    // 强调色按下（偏 16%）
    Color control_disabled = 0;  // 控件禁用底：surface 向 panel 混 60%
    Color scrollbar_thumb = 0;   // 滚动条滑块：border_strong 向背景收 35%
};

// 令牌 → 已解析主题。tokens.parse 已保证结构合法，这里只做派生，恒成功。
ResolvedTheme resolve(const ThemeTokens& tokens);

// 编译期内嵌的默认主题数据（构建时从 design/*.json 生成，勿手改）。
// 运行期不依赖任何包安装路径；外部 json 只作为换肤/覆盖入口。
namespace embed {
const char* light_json();
const char* dark_json();
const char* icons_json();
}  // namespace embed

// 内嵌默认主题（首次访问解析一次；内嵌数据受单测保护，恒可解析）。
const ResolvedTheme& LightTheme();
const ResolvedTheme& DarkTheme();
const IconMap& DefaultIcons();

// 进程内活动主题。duilib 层控件绘制时读取；切主题 = Set 后对控件树整体 Invalidate。
const ResolvedTheme& ActiveTheme();
void SetActiveTheme(const ResolvedTheme& theme);

}  // namespace uikit
