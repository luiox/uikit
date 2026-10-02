#include "uikit/duilib/combobox.h"

namespace uikit {

using DuiLib::CDuiRect;
using DuiLib::CDuiString;
using DuiLib::CControlUI;
using DuiLib::UIRender;

ComboBox::ComboBox() {
    const ResolvedTheme& t = ActiveTheme();
    SetFixedHeight(t.metrics.control_height);
    // 影窗依赖图片资产（fork 缺口），直角密度下用 1px 描边分层即可。
    SetShowShadow(false);
    DuiLib::CDuiString padding;
    padding.Format(_T("%d,0,%d,0"), t.metrics.control_hpad + 4, t.metrics.control_hpad);
    SetAttribute(_T("textpadding"), padding.GetData());
    OnThemeChanged();
}

void ComboBox::OnThemeChanged() {
    const ResolvedTheme& t = ActiveTheme();
    SetBkColor(t.color.surface);
    SetAttribute(_T("bordercolor"), duilib::attr(t.color.border));
    SetAttribute(_T("bordersize"), _T("1"));
    // 闭合态文本走控件级颜色（CComboUI::PaintText 的 GetTextColor 链）。
    SetTextColor(t.color.text);

    auto* info = GetListInfo();
    // 弹层条目配色：surface 实底（无图资产下的弹层底），悬停/选中走 hover/selected。
    info->SetItemTextColor(t.color.text);
    info->SetSelectedItemTextColor(t.color.text);
    info->SetHotItemTextColor(t.color.text);
    info->SetDisabledItemTextColor(t.control_disabled);
    info->SetItemBkColor(t.color.surface);
    info->SetSelectedItemBkColor(t.color.surface_selected);
    info->SetHotItemBkColor(t.color.surface_hover);
    info->SetItemShowRowLine(false);
    SetTipValueColor(DuiLib::CDuiColor(t.text_secondary));
}

DuiLib::CControlUI* ComboBox::AddOption(LPCTSTR text, UINT_PTR item_data) {
    auto* item = AddString(text, item_data);
    if (item != nullptr) {
        item->SetFixedHeight(ActiveTheme().metrics.control_height);
    }
    return item;
}

bool ComboBox::DoPaint(UIRender* pRender, const CDuiRect& rcPaint, CControlUI* pStopControl) {
    const bool ok = CComboUI::DoPaint(pRender, rcPaint, pStopControl);
    // 自绘下拉箭头（fork 的 db 系列走图片资产，无资源时什么都不画）。
    const ResolvedTheme& t = ActiveTheme();
    const CDuiRect rb = GetDropButtonRect();
    const int w = rb.right - rb.left;
    const int h = rb.bottom - rb.top;
    if (w <= 0 || h <= 0) {
        return ok;
    }
    Color stroke_color = t.text_secondary;
    if (!IsEnabled()) {
        stroke_color = t.control_disabled;
    } else if (IsHotState() || IsPushedState()) {
        stroke_color = t.color.text;
    }
    const int stroke = duilib::scale(m_pManager, 2);
    const int cx = rb.left + w / 2;
    pRender->DrawLine(cx - w * 22 / 100, rb.top + h * 40 / 100, cx, rb.top + h * 62 / 100,
                      stroke, stroke_color);
    pRender->DrawLine(cx, rb.top + h * 62 / 100, cx + w * 22 / 100, rb.top + h * 40 / 100,
                      stroke, stroke_color);
    return ok;
}

}  // namespace uikit
