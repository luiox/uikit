#include "uikit/duilib/slider.h"

namespace uikit {

using DuiLib::CDuiRect;
using DuiLib::CDuiSize;
using DuiLib::UIRender;

// 96 基准几何：滑块直径。
constexpr int kThumbDiameter = 14;

Slider::Slider() {
    SetFixedHeight(24);
    SetThumbSize(DuiLib::CDuiSize(kThumbDiameter, kThumbDiameter));
    SetMinValue(0);
    SetMaxValue(100);
    SetValue(0);
    SetAttribute(_T("bordersize"), _T("0"));
}

void Slider::PaintForeImage(UIRender* pRender) {
    const ResolvedTheme& t = ActiveTheme();
    const int h = m_rcItem.bottom - m_rcItem.top;
    const int w = m_rcItem.right - m_rcItem.left;
    if (h <= 0 || w <= 0) {
        return;
    }
    const bool enabled = IsEnabled();

    // 滑块：直径/行程与 fork 的 GetThumbRect（DoEvent 拖拽命中）保持一致——
    // 同为 96 基准直径按 DPI 缩放、行程占满扣去直径后的宽度。
    const int knob = duilib::scale(m_pManager, kThumbDiameter);
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
    const int knob_x = m_rcItem.left + (w - knob) * (value - GetMinValue()) / span;
    const int knob_y = m_rcItem.top + (h - knob) / 2;
    const CDuiRect knob_rect{knob_x, knob_y, knob_x + knob, knob_y + knob};

    // 轨道：4px 直角条竖直居中；已选段到滑块圆心为止。
    const int track_h = duilib::scale(m_pManager, kTrackHeight);
    const int track_y = m_rcItem.top + (h - track_h) / 2;
    const CDuiRect track{m_rcItem.left, track_y, m_rcItem.right, track_y + track_h};
    int radius = duilib::scale(m_pManager, t.metrics.radius_control);
    if (radius > track_h / 2) {
        radius = track_h / 2;
    }
    const Color track_color = enabled ? t.color.surface : t.control_disabled;
    pRender->DrawColor(track, CDuiSize(radius, radius), track_color);

    const int fill_w = (knob_x + knob / 2) - m_rcItem.left;
    if (fill_w > 0) {
        const CDuiRect fill{m_rcItem.left, track_y, m_rcItem.left + fill_w, track_y + track_h};
        const int fr = (fill_w < radius) ? fill_w : radius;
        pRender->DrawColor(fill, CDuiSize(fr, fr), enabled ? t.color.accent : t.control_disabled);
    }

    // 滑块：accent 圆点 + on_accent 细环（悬停/按下环色加深）。
    const int ring_r = knob / 2;
    pRender->DrawColor(knob_rect, CDuiSize(ring_r, ring_r),
                       enabled ? t.color.accent : t.control_disabled);
    const int ring_w = duilib::scale(m_pManager, 2);
    const Color ring_color = !enabled     ? t.control_disabled
                             : IsCaptureState() || IsHotState() ? t.color.border_focus
                                                                : t.color.on_accent;
    pRender->DrawEllipse(knob_rect, ring_w, ring_color);
}

}  // namespace uikit
