#pragma once

// 步进器（L3，uikit 顶层命名空间）：[−] 值 [+] 的数值微调复合控件。

#include <UIlib.h>

#include "uikit/duilib/controls.h"
#include "uikit/duilib/edit.h"

namespace uikit {

// 复合布局：减钮 + Edit + 加钮（行高 control_height）。−/+ 点击自持（不经
// 窗口 Notify 也能用），Edit 手输经 fork 的控件级 OnNotify 挂点内部同步
// （return/killfocus 时解析钳位）。每次值变化对窗口发 "valuechange" 通知
// （wParam = 新值），宿主按需处理。
class SpinBox : public DuiLib::CHorizontalLayoutUI, public duilib::ThemeableControl {
public:
    SpinBox();
    LPCTSTR GetClass() const override { return _T("UIKitSpinBox"); }
    void OnThemeChanged() override;

    void SetValue(int value);
    int GetValue() const { return value_; }
    void SetValueRange(int min_value, int max_value);
    void SetStep(int step) { step_ = (step > 0) ? step : 1; }

protected:
    // 子控件通知挂点（fork 的 CControlUI::OnNotify 事件源）。
    bool OnChildNotify(void* param);

private:
    class StepButton;
    void Step(int direction);
    void ApplyValue();

    Edit* edit_ = nullptr;
    int value_ = 0;
    int min_value_ = 0;
    int max_value_ = 100;
    int step_ = 1;
};

}  // namespace uikit
