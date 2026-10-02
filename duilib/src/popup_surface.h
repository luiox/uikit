#pragma once

// 弹层根面板（L3 内部共用件，不安装）：Menu 弹层与 DatePicker 日历弹层共用的
// surface 底 + 1px 描边。圆角一律走 radius_control 令牌（默认 0 = 直角密度），
// 不在控件里写死外观；换主题即整体跟随。

#include <UIlib.h>

#include "uikit/duilib/compat.h"
#include "uikit/duilib/controls.h"
#include "uikit/theme/theme.h"

namespace uikit::detail {

class PopupSurfaceUI : public duilib::PanelUI {
public:
    LPCTSTR GetClass() const override { return _T("UIKitPopupSurface"); }

    // 底色覆盖为 surface（PanelUI 默认 panel——弹层是浮出的操作面，与输入框
    // 同族），直读主题不走 bkcolor 属性，主题切换下一帧自动跟随。
    void PaintBkColor(DuiLib::UIRender* pRender) override {
        const ResolvedTheme& t = ActiveTheme();
        const int r = duilib::scale(m_pManager, t.metrics.radius_control);
        pRender->DrawColor(m_rcItem, DuiLib::CDuiSize(r, r), t.color.surface);
    }

    // 1px 描边（Edit::PaintBorder 同款画法），圆角同样取令牌。
    void PaintBorder(DuiLib::UIRender* pRender) override {
        const ResolvedTheme& t = ActiveTheme();
        const int r = duilib::scale(m_pManager, t.metrics.radius_control);
        const DuiLib::CDuiSize round(r, r);
        pRender->DrawRoundRect(m_rcItem, duilib::scale(m_pManager, 1), round,
                               t.color.border);
    }
};

}  // namespace uikit::detail
