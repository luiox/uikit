#pragma once

// 进度条（L3，uikit 顶层命名空间）：surface 直角轨道 + accent 填充。

#include <UIlib.h>

#include "uikit/duilib/controls.h"

namespace uikit {

// 横向进度条：主题色自绘——surface 轨道 + accent 填充，圆角随 radius_control
// 令牌（默认 0 = 直角，密度优先），高度 6 逻辑像素（细条现代样式，容器自行
// 留行距）。min/max/value 语义沿 fork 的 CProgressUI（SetValue 不钳位，越界
// 值被拒收，调用方自行夹取）。
class ProgressBar : public DuiLib::CProgressUI, public duilib::ThemeableControl {
public:
    ProgressBar();
    LPCTSTR GetClass() const override { return _T("UIKitProgressBar"); }

protected:
    // 轨道 + 比例填充一并绘制（不依赖 foreimage/bkcolor 属性）。
    void PaintForeColor(DuiLib::UIRender* pRender) override;
};

}  // namespace uikit
