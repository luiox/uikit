#include "uikit/duilib/progress.h"

namespace uikit {

using DuiLib::CDuiRect;
using DuiLib::CDuiSize;
using DuiLib::UIRender;

ProgressBar::ProgressBar() {
    SetMinValue(0);
    SetMaxValue(100);
    SetValue(0);
    SetFixedHeight(6);
}

void ProgressBar::PaintForeColor(UIRender* pRender) {
    const ResolvedTheme& t = ActiveTheme();
    const int h = m_rcItem.bottom - m_rcItem.top;
    if (h <= 0) {
        return;
    }
    // 圆角随主题令牌（默认直角）；上限半高，避免 GDI 圆角大于半宽/半高。
    int radius = duilib::scale(m_pManager, t.metrics.radius_control);
    if (radius > h / 2) {
        radius = h / 2;
    }
    // 轨道：surface（深浅主题下都与 panel/surface 底形成对比）。
    pRender->DrawColor(m_rcItem, CDuiSize(radius, radius), t.color.surface);

    int span = GetMaxValue() - GetMinValue();
    if (span <= 0) {
        span = 1;
    }
    int value = GetValue();
    if (value < GetMinValue()) {
        value = GetMinValue();
    }
    if (value > GetMaxValue()) {
        value = GetMaxValue();
    }
    const int width = (m_rcItem.right - m_rcItem.left) * (value - GetMinValue()) / span;
    if (width <= 0) {
        return;
    }
    const int fr = (width < radius) ? width : radius;  // 半程收窄，同上
    const CDuiRect fill{m_rcItem.left, m_rcItem.top, m_rcItem.left + width, m_rcItem.bottom};
    pRender->DrawColor(fill, CDuiSize(fr, fr), t.color.accent);
}

}  // namespace uikit
