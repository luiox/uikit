#pragma once

// 下拉选择（L3，uikit 顶层命名空间）。

#include <UIlib.h>

#include "uikit/duilib/controls.h"

namespace uikit {

// 主题化下拉框（fork 的 CComboUI 承接弹出/选择/下拉编辑语义，本层只做主题
// 化）：surface 直角底 + 状态边框（同 Edit），条目/弹层配色经 TListInfoUI
// 实时取主题。箭头为自绘双线（无图片资产）。CBS_DROPDOWN（可编辑）等原生
// 能力沿 CComboUI::SetDropType；条目经 AddString 挂入。
class ComboBox : public DuiLib::CComboUI, public duilib::ThemeableControl {
public:
    ComboBox();
    LPCTSTR GetClass() const override { return _T("UIKitComboBox"); }
    void OnThemeChanged() override;

    // 条目便捷挂入：行高对齐 control_height（AddString 直挂的条目高随文本）。
    DuiLib::CControlUI* AddOption(LPCTSTR text, UINT_PTR item_data = 0);

protected:
    // base DoPaint 后补画自绘下拉箭头（无图片时的替代，见 fork 缺口说明）。
    bool DoPaint(DuiLib::UIRender* pRender, const DuiLib::CDuiRect& rcPaint,
                 DuiLib::CControlUI* pStopControl) override;
};

}  // namespace uikit
