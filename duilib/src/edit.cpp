#include "uikit/duilib/edit.h"

namespace uikit {

using DuiLib::UIRender;

Edit::Edit() {
    const ResolvedTheme& t = ActiveTheme();
    SetFixedHeight(t.metrics.control_height);
    DuiLib::CDuiString padding;
    padding.Format(_T("%d,4,%d,4"), t.metrics.control_hpad, t.metrics.control_hpad);
    SetAttribute(_T("textpadding"), padding.GetData());
    SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    OnThemeChanged();
}

void Edit::OnThemeChanged() {
    const ResolvedTheme& t = ActiveTheme();
    SetTextColor(t.color.text);
    SetBkColor(t.color.surface);
    // 占位文本（fork 的 tip 机制）实时取弱化色。
    SetTipValueColor(DuiLib::CDuiColor(t.text_secondary));
    // native 刷子不动（SearchBoxUI 同款约束——会令原生 EDIT 底发黑）。
}

void Edit::PaintBkColor(UIRender* pRender) {
    // 底改主题圆角绘制（base 的 bkcolor 方块填充不用；bkcolor 值仍留给 native 链路）。
    // radius_control 令牌默认 0（直角密度优先），主题给正值即整体圆角。
    const ResolvedTheme& t = ActiveTheme();
    const int r = duilib::scale(m_pManager, t.metrics.radius_control);
    pRender->DrawColor(m_rcItem, DuiLib::CDuiSize(r, r), t.color.surface);
}

void Edit::PaintBorder(UIRender* pRender) {
    const ResolvedTheme& t = ActiveTheme();
    const int r = duilib::scale(m_pManager, t.metrics.radius_control);
    const DuiLib::CDuiSize round(r, r);
    Color outline = t.color.border;
    if (!IsEnabled()) {
        outline = t.control_disabled;
    } else if (IsFocused()) {
        outline = t.color.accent;
    } else if (IsHotState()) {
        outline = t.color.border_focus;
    }
    pRender->DrawRoundRect(m_rcItem, duilib::scale(m_pManager, 1), round, outline);
}

}  // namespace uikit
