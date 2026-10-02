#include "uikit/duilib/switch.h"

namespace uikit {

using DuiLib::CDuiRect;
using DuiLib::CDuiSize;
using DuiLib::UIRender;

// 96 基准几何：轨道高/宽、滑块与轨道的内边距。
constexpr int kTrackHeight = 20;
constexpr int kTrackWidth = 36;
constexpr int kKnobInset = 3;

Switch::Switch() {
    SetFixedHeight(kTrackHeight);
    SetFixedWidth(kTrackWidth);
    // 透明底按钮（ButtonUI 预设），视觉完全自绘。
    SetStateColors(color::kTransparent, color::kTransparent, color::kTransparent);
    SetAttribute(_T("bordercolor"), _T("0x00000000"));
    SetAttribute(_T("bordersize"), _T("0"));
}

void Switch::SetChecked(bool checked) {
    if (checked_ == checked) {
        return;
    }
    checked_ = checked;
    Invalidate();
}

bool Switch::Activate() {
    if (!ButtonUI::Activate()) {
        return false;
    }
    checked_ = !checked_;
    Invalidate();
    return true;
}

void Switch::PaintStatusImage(UIRender* pRender) {
    const ResolvedTheme& t = ActiveTheme();
    const bool enabled = IsEnabled();
    const int h = m_rcItem.bottom - m_rcItem.top;
    const int w = m_rcItem.right - m_rcItem.left;
    if (h <= 0 || w <= 0) {
        return;
    }
    // 滑块直径与行程按实际轨道尺寸比例缩放（控件可被布局拉伸）。
    const int knob = h - duilib::scale(m_pManager, kKnobInset) * 2;
    const int travel = w - knob - duilib::scale(m_pManager, kKnobInset) * 2;
    int radius = duilib::scale(m_pManager, t.metrics.radius_control);
    if (radius > h / 2) {
        radius = h / 2;
    }

    const bool hot = IsHotState() || IsPushedState();
    Color track = t.color.border_strong;
    if (checked_) {
        track = enabled ? t.color.accent : t.control_disabled;
    } else {
        track = !enabled ? t.control_disabled : (hot ? t.color.border_focus : t.color.border_strong);
    }
    Color knob_color = checked_ ? t.color.on_accent : t.color.surface;
    if (!enabled) {
        knob_color = t.color.panel;
    }

    pRender->DrawColor(m_rcItem, CDuiSize(radius, radius), track);

    // 滑块行程：关位贴左内边距，开位贴右；圆点带 1px 主色描边提升对比。
    const int inset = duilib::scale(m_pManager, kKnobInset);
    const int x = checked_ ? (w - inset - knob) : inset;
    const CDuiRect knob_rect{m_rcItem.left + x, m_rcItem.top + inset,
                             m_rcItem.left + x + knob, m_rcItem.top + inset + knob};
    const int ring = (radius > 0) ? knob / 2 : 0;
    pRender->DrawColor(knob_rect, CDuiSize(ring, ring), knob_color);
}

}  // namespace uikit
