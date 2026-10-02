#pragma once

// 滑杆（L3，uikit 顶层命名空间）：连续值选择。

#include <UIlib.h>

#include "uikit/duilib/controls.h"

namespace uikit {

// 主题色自绘滑杆：surface 直角轨道（4px）+ accent 已选段 + accent 圆形滑块
// （on_accent 描边）。拖拽/点击定位与 DPI 缩放由 fork 的 CSliderUI 承接
// （SetThumbSize 与视觉一致以保命中区域正确）；圆角随 radius_control 令牌。
// 高度默认 24（96 基准），容器自行留行距。value 语义沿 CProgressUI。
class Slider : public DuiLib::CSliderUI, public duilib::ThemeableControl {
public:
    Slider();
    LPCTSTR GetClass() const override { return _T("UIKitSlider"); }

protected:
    // 轨道/已选段/滑块一并绘制（不依赖 foreimage/thumbimage 属性）。
    void PaintForeImage(DuiLib::UIRender* pRender) override;

private:
    static constexpr int kTrackHeight = 4;  // 96 基准轨道厚
};

}  // namespace uikit
