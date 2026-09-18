#pragma once

#include <string>
#include <vector>

#include "uikit/theme/tokens.h"
// 内嵌数据直接随头给出（inline 定义进每个使用 TU）：GCC 对「先非 inline 声明、
// 后 inline 定义」的分头写法不做跨 TU 符号发射，链接期 undefined reference
//（CI linux 门禁实测），MSVC 则宽容。单头可见是跨编译器唯一稳态。
#include "uikit/embedded_themes.h"

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

// 编译期内嵌的默认主题数据（light/dark/icons_json，定义见 embedded_themes.h，
// 由 tools/gen_embed.sh 生成；运行期不依赖任何包安装路径，外部 json 只作为
// 换肤/覆盖入口）。声明随上面的 include 自带，勿在此重复。

// 内嵌默认主题（首次访问解析一次；内嵌数据受单测保护，恒可解析）。
const ResolvedTheme& LightTheme();
const ResolvedTheme& DarkTheme();
const IconMap& DefaultIcons();

// 进程内活动主题。duilib 层控件绘制时读取；切主题 = Set 后对控件树整体 Invalidate。
const ResolvedTheme& ActiveTheme();
void SetActiveTheme(const ResolvedTheme& theme);

}  // namespace uikit
