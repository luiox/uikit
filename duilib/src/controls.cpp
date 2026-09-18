#include "uikit/duilib/controls.h"

#include <cstddef>
#include <vector>

namespace uikit::duilib {

using DuiLib::CDuiRect;
using DuiLib::CDuiSize;
using DuiLib::CDuiString;
using DuiLib::CButtonUI;
using DuiLib::CControlUI;
using DuiLib::UIRender;

// —— LabelUI ————————————————————————————————————————————————

void LabelUI::PaintText(UIRender* pRender) {
    const ResolvedTheme& t = ActiveTheme();
    CLabelUI::SetTextColor(role_ == Role::Secondary ? t.text_secondary : t.color.text);
    CLabelUI::PaintText(pRender);
}

void PanelUI::OnThemeChanged() {
    SetBkColor(ActiveTheme().color.panel);
}

// —— ButtonUI ————————————————————————————————————————————————

void ButtonUI::SetStateColors(Color normal, Color hot, Color pushed) {
    style_ = Style::Custom;
    custom_normal_ = normal;
    custom_hot_ = hot;
    custom_pushed_ = pushed;
}

void ButtonUI::SetActive(bool active) {
    if (active_ == active) {
        return;
    }
    active_ = active;
    Invalidate();
}

void ButtonUI::StylePrimary() {
    style_ = Style::Primary;
    SetTextColor(ActiveTheme().color.on_accent);
    SetAttribute(_T("bordercolor"), _T("0x00000000"));
    SetAttribute(_T("bordersize"), _T("0"));
}

void ButtonUI::StyleSecondary() {
    style_ = Style::Secondary;
    SetTextColor(ActiveTheme().color.text);
    SetAttribute(_T("bordercolor"), _T("0x00000000"));
    SetAttribute(_T("bordersize"), _T("0"));
}

void ButtonUI::ResolveStateColors(Color& normal, Color& hot, Color& pushed) const {
    const ResolvedTheme& t = ActiveTheme();
    switch (style_) {
        case Style::Primary:
            normal = t.color.accent;
            hot = t.accent_hover;
            pushed = t.accent_pressed;
            break;
        case Style::Secondary:
            normal = t.color.panel;
            hot = t.color.button_hover;
            pushed = t.color.panel_hover;
            break;
        case Style::Custom:
            normal = custom_normal_;
            hot = custom_hot_;
            pushed = custom_pushed_;
            break;
        case Style::Transparent:
        default:
            normal = hot = pushed = color::kTransparent;
            break;
    }
}

void ButtonUI::PaintStatusImage(UIRender* pRender) {
    const ResolvedTheme& t = ActiveTheme();
    Color fill_normal = color::kTransparent;
    Color fill_hot = color::kTransparent;
    Color fill_pushed = color::kTransparent;
    ResolveStateColors(fill_normal, fill_hot, fill_pushed);

    Color fill = fill_normal;
    if (active_ && fill_pushed != color::kTransparent) {
        fill = fill_pushed;
    } else if (IsPushedState() && fill_pushed != color::kTransparent) {
        fill = fill_pushed;
    } else if (IsHotState() && fill_hot != color::kTransparent) {
        fill = fill_hot;
    }
    // 禁用态：样式按钮退到禁用底色；透明底按钮维持透明。
    if (!IsEnabled() && fill_normal != color::kTransparent) {
        fill = t.control_disabled;
    }
    if (fill != color::kTransparent) {
        const int r = scale(m_pManager, t.metrics.radius_control);
        pRender->DrawColor(m_rcItem, CDuiSize(r, r), fill);
    }
    CButtonUI::PaintStatusImage(pRender);
}

void ButtonUI::PaintText(DuiLib::UIRender* pRender) {
    // 文本色随主题实时解析（SetTextColor 无副作用，只是成员赋值）。
    const ResolvedTheme& t = ActiveTheme();
    if (style_ == Style::Primary) {
        CButtonUI::SetTextColor(t.color.on_accent);
    } else if (style_ == Style::Secondary) {
        CButtonUI::SetTextColor(t.color.text);
    }
    CButtonUI::PaintText(pRender);
}

IconButtonUI::IconButtonUI() {
    SetText(_T(""));
    // 顶栏按钮统一等大正方形，常态透明，悬停才显示灰底。
    SetFixedWidth(ActiveTheme().metrics.icon_button_size);
    SetFixedHeight(ActiveTheme().metrics.icon_button_size);
}

void IconButtonUI::SetSvgImage(const CDuiString& image_attr) {
    SetAttribute(_T("normalimage"), image_attr.GetData());
    SetAttribute(_T("hotimage"), image_attr.GetData());
    SetAttribute(_T("pushedimage"), image_attr.GetData());
}

DuiLib::CButtonUI* MakeTextButton(LPCTSTR name, LPCTSTR text, int width) {
    auto* button = new ButtonUI();
    button->SetName(name);
    button->SetText(text);
    if (width > 0) {
        button->SetFixedWidth(width);
    }
    button->SetFixedHeight(ActiveTheme().metrics.control_height);
    button->StyleSecondary();
    return button;
}

// —— CheckBoxUI ——————————————————————————————————————————————

CheckBoxUI::CheckBoxUI() {
    const ResolvedTheme& t = ActiveTheme();
    SetFixedHeight(t.metrics.control_height);
    SetStateColors(color::kTransparent, color::kTransparent, color::kTransparent);
    SetAttribute(_T("bordercolor"), _T("0x00000000"));
    SetAttribute(_T("bordersize"), _T("0"));
    // 文本让位勾选盒（盒 checkbox_box + 8px 间距）。
    DuiLib::CDuiString text_padding;
    text_padding.Format(_T("%d,0,4,0"), t.metrics.checkbox_box + 8);
    SetAttribute(_T("textpadding"), text_padding.GetData());
    SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void CheckBoxUI::SetChecked(bool checked) {
    if (checked_ == checked) {
        return;
    }
    checked_ = checked;
    Invalidate();
}

bool CheckBoxUI::Activate() {
    if (!ButtonUI::Activate()) {
        return false;
    }
    checked_ = !checked_;
    Invalidate();
    return true;
}

void CheckBoxUI::PaintStatusImage(UIRender* pRender) {
    ButtonUI::PaintStatusImage(pRender);

    const ResolvedTheme& t = ActiveTheme();
    // 勾选盒按 DPI 缩放（checkbox_box 为 96 基准），对勾端点按盒尺寸比例定位。
    const int box_size = scale(m_pManager, t.metrics.checkbox_box);
    const int stroke = scale(m_pManager, t.metrics.check_stroke);
    const int border = scale(m_pManager, 1);
    const int radius = scale(m_pManager, t.metrics.radius_checkbox);
    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    const CDuiRect box{m_rcItem.left, cy - box_size / 2, m_rcItem.left + box_size,
                       cy + box_size / 2};
    const bool hot = IsHotState() || IsPushedState();
    const bool enabled = IsEnabled();

    // 禁用态整体退色（底/描边都向背景收）。
    const Color fill = !enabled   ? t.control_disabled
                       : checked_ ? t.color.accent
                                  : t.color.surface;
    const Color outline = !enabled ? t.control_disabled
                          : checked_ ? t.color.accent
                                     : (hot ? t.color.border_focus : t.color.border_strong);
    const Color mark = checked_ ? (enabled ? t.color.on_accent : t.color.panel) : color::kTransparent;

    const CDuiSize round(radius, radius);
    pRender->DrawColor(box, round, fill);
    pRender->DrawRoundRect(box, border, round, outline);
    if (checked_) {
        // 对勾（两段线，短撇 + 长捺），端点为基准盒内的比例坐标。
        pRender->DrawLine(box.left + box_size * 3 / 14, box.top + box_size * 8 / 14,
                          box.left + box_size * 6 / 14, box.top + box_size * 10 / 14, stroke,
                          mark);
        pRender->DrawLine(box.left + box_size * 6 / 14, box.top + box_size * 10 / 14,
                          box.left + box_size * 11 / 14, box.top + box_size * 3 / 14, stroke,
                          mark);
    }
}

// —— RadioButtonUI ———————————————————————————————————————————

RadioButtonUI::RadioButtonUI() {
    const ResolvedTheme& t = ActiveTheme();
    SetFixedHeight(t.metrics.control_height);
    SetStateColors(color::kTransparent, color::kTransparent, color::kTransparent);
    SetAttribute(_T("bordercolor"), _T("0x00000000"));
    SetAttribute(_T("bordersize"), _T("0"));
    DuiLib::CDuiString text_padding;
    text_padding.Format(_T("%d,0,4,0"), t.metrics.checkbox_box + 8);
    SetAttribute(_T("textpadding"), text_padding.GetData());
    SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void RadioButtonUI::SetChecked(bool checked) {
    if (checked_ == checked) {
        return;
    }
    checked_ = checked;
    Invalidate();
}

void RadioButtonUI::SetGroup(CControlUI* const* siblings, size_t count) {
    siblings_.assign(siblings, siblings + count);
}

bool RadioButtonUI::Activate() {
    if (!ButtonUI::Activate()) {
        return false;
    }
    UncheckSiblings();
    checked_ = true;
    Invalidate();
    return true;
}

void RadioButtonUI::UncheckSiblings() {
    for (auto* peer : siblings_) {
        if (peer == this) {
            continue;
        }
        if (auto* radio = dynamic_cast<RadioButtonUI*>(peer)) {
            radio->SetChecked(false);
        }
    }
}

void RadioButtonUI::PaintStatusImage(UIRender* pRender) {
    ButtonUI::PaintStatusImage(pRender);

    const ResolvedTheme& t = ActiveTheme();
    const int box_size = scale(m_pManager, t.metrics.checkbox_box);
    const int stroke = scale(m_pManager, 2);
    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    const CDuiRect ring{m_rcItem.left, cy - box_size / 2, m_rcItem.left + box_size,
                        cy + box_size / 2};
    const bool hot = IsHotState() || IsPushedState();
    const bool enabled = IsEnabled();

    const Color outline = !enabled ? t.control_disabled
                          : checked_ ? t.color.accent
                                     : (hot ? t.color.border_focus : t.color.border_strong);
    pRender->DrawEllipse(ring, stroke, outline);
    if (checked_) {
        // 内点直径 = 盒的 45%。
        const int dot = box_size * 9 / 20;
        const CDuiRect dot_rect{ring.left + (box_size - dot) / 2, (ring.top + ring.bottom) / 2 - dot / 2,
                                ring.left + (box_size - dot) / 2 + dot,
                                (ring.top + ring.bottom) / 2 + dot / 2};
        pRender->FillEllipse(dot_rect, enabled ? t.color.accent : t.control_disabled);
    }
}

// —— 列表族 ————————————————————————————————————————————————

GroupListUI::GroupListUI() {
    // CListUI 默认带 Header，占位会造成顶部"空白条"，强制隐藏（切主题不还原）。
    if (GetHeader() != nullptr) {
        GetHeader()->SetVisible(false);
    }
    SetChildPadding(0);
    // 选中条右缘与条目区之间留 1px 背景缝隙（条带宽 = 面板宽 - 1）。
    SetAttribute(_T("inset"), _T("0,0,1,0"));
    SetAttribute(_T("bordersize"), _T("0"));
    OnThemeChanged();
}

void GroupListUI::OnThemeChanged() {
    const ResolvedTheme& t = ActiveTheme();
    SetBkColor(t.color.panel);
    auto* list_info = GetListInfo();
    list_info->SetItemBkColor(color::kTransparent);
    list_info->SetSelectedItemBkColor(t.color.panel_selected);
    list_info->SetHotItemBkColor(t.color.panel_hover);
    SetAttribute(_T("bordercolor"), attr(t.color.divider));
}

ItemListUI::ItemListUI() {
    if (GetHeader() != nullptr) {
        GetHeader()->SetVisible(false);
    }
    SetChildPadding(0);
    SetAttribute(_T("inset"), _T("0,0,0,0"));
    SetAttribute(_T("bordersize"), _T("0"));
    OnThemeChanged();
}

void ItemListUI::OnThemeChanged() {
    const ResolvedTheme& t = ActiveTheme();
    SetBkColor(t.color.surface);
    auto* list_info = GetListInfo();
    list_info->SetItemBkColor(color::kTransparent);
    list_info->SetSelectedItemBkColor(t.color.surface_selected);
    list_info->SetHotItemBkColor(t.color.surface_hover);
    SetAttribute(_T("bordercolor"), attr(t.color.border));
}

bool GroupRowUI::DoPaint(UIRender* pRender, const CDuiRect& rcPaint, CControlUI* pStopControl) {
    const bool handled = CListContainerElementUI::DoPaint(pRender, rcPaint, pStopControl);
    if (IsSelected()) {
        pRender->DrawRect(m_rcItem, 1, ActiveTheme().color.row_selected_border);
    }
    return handled;
}

void ApplyFlatScrollbar(DuiLib::CListUI* list, Color track_color,
                        const DuiLib::CDuiString& thumb_attr) {
    if (list == nullptr) {
        return;
    }
    list->EnableScrollBar(true, false);
    auto* vsb = list->GetVerticalScrollBar();
    if (vsb == nullptr) {
        return;
    }
    vsb->SetFixedWidth(ActiveTheme().metrics.scrollbar_width);
    vsb->SetBkColor(track_color);
    vsb->SetShowButton1(false);
    vsb->SetShowButton2(false);
    if (!thumb_attr.IsEmpty()) {
        vsb->SetThumbNormalImage(thumb_attr.GetData());
        vsb->SetThumbHotImage(thumb_attr.GetData());
        vsb->SetThumbPushedImage(thumb_attr.GetData());
    }
}

// —— 输入/顶栏 ————————————————————————————————————————————

SearchBoxUI::SearchBoxUI() {
    const ResolvedTheme& t = ActiveTheme();
    SetFixedHeight(t.metrics.search_height);
    DuiLib::CDuiString padding;
    padding.Format(_T("%d,4,%d,4"), t.metrics.control_hpad, t.metrics.control_hpad);
    SetAttribute(_T("textpadding"), padding.GetData());
    SetAttribute(_T("bordersize"), _T("0"));
    OnThemeChanged();
}

void SearchBoxUI::OnThemeChanged() {
    const ResolvedTheme& t = ActiveTheme();
    SetTextColor(t.color.text);
    // 输入区自身 surface 底、无边框；native 刷子不动（会令原生 EDIT 底发黑）。
    SetBkColor(t.color.surface);
    SetAttribute(_T("bordercolor"), attr(t.color.surface));
}

TitleBarUI::TitleBarUI() {
    const ResolvedTheme& t = ActiveTheme();
    SetFixedHeight(t.metrics.titlebar_height);
    // 按钮在顶栏内垂直居中，标题拉伸占满整行。
    SetAttribute(_T("childvalign"), _T("center"));
    SetAttribute(_T("childpadding"), _T("0"));
    DuiLib::CDuiString inset;
    inset.Format(_T("%d,0,0,0"), t.metrics.window_inset);
    SetAttribute(_T("inset"), inset.GetData());
    SetAttribute(_T("bordersize"), _T("0,0,0,0"));
    OnThemeChanged();
}

void TitleBarUI::OnThemeChanged() {
    SetBkColor(ActiveTheme().color.panel);
    SetAttribute(_T("bordercolor"), attr(ActiveTheme().color.border));
}

}  // namespace uikit::duilib
