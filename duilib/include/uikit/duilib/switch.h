#pragma once

// 开关（L3，uikit 顶层命名空间）：滑块式二态切换。

#include <UIlib.h>

#include "uikit/duilib/controls.h"

namespace uikit {

// 自绘开关：轨道圆角随 radius_control 令牌（默认直角密度优先），开=accent 底
// + on_accent 滑块，关=border_strong 底；悬停加深、禁用整体退色。点击自切换
// 并发 click 通知（IsChecked 读态），与 CheckBoxUI 同一套交互语义。
// 几何：轨道高 20、宽 36（96 基准，绘制时按 DPI 缩放）。
class Switch : public duilib::ButtonUI {
public:
    Switch();
    LPCTSTR GetClass() const override { return _T("UIKitSwitch"); }
    void SetChecked(bool checked);
    bool IsChecked() const { return checked_; }

protected:
    bool Activate() override;
    void PaintStatusImage(DuiLib::UIRender* pRender) override;

private:
    bool checked_ = false;
};

}  // namespace uikit
