#pragma once

// 输入框（L3，uikit 顶层命名空间）：surface 直角底 + 1px 状态边框。
// 命名约定：面向消费方的新式控件直接挂 uikit（无 UI 后缀，uikit::Edit）；
// uikit::duilib 保留给包装层细节与既有控件。

#include <UIlib.h>

#include "uikit/duilib/controls.h"

namespace uikit {

// 主题化输入框：surface 直角底 + 状态边框（常态 border → 悬停 border_focus
// → 聚焦 accent → 禁用 control_disabled），占位文本走 fork 的 tip 机制（颜色
// 实时取主题 text_secondary）。单行/密码/纯数字等原生能力由 CEditUI 承接，
// 消费方按需 SetPasswordMode/SetNumberOnly/SetMultiLine。
class Edit : public DuiLib::CEditUI, public duilib::ThemeableControl {
public:
    Edit();
    LPCTSTR GetClass() const override { return _T("UIKitEdit"); }
    void OnThemeChanged() override;

protected:
    void PaintBkColor(DuiLib::UIRender* pRender) override;
    void PaintBorder(DuiLib::UIRender* pRender) override;
};

}  // namespace uikit
