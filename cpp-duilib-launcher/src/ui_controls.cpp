#include "ui_controls.h"

using namespace DuiLib;

namespace appui {

const Theme& GetTheme() {
    static const Theme theme{};
    return theme;
}

IconButtonUI::IconButtonUI() {
    const Theme& theme = GetTheme();
    SetText(_T(""));
    SetFixedWidth(26);
    SetFixedHeight(26);
    SetTextColor(theme.text);
    SetAttribute(_T("normalbkcolor"), theme.icon_btn_normal);
    SetAttribute(_T("hotbkcolor"), theme.icon_btn_hot);
    SetAttribute(_T("pushedbkcolor"), theme.icon_btn_pushed);
    SetAttribute(_T("bordercolor"), theme.border_color);
    SetAttribute(_T("bordersize"), theme.border_one);
}

void IconButtonUI::SetSvgImage(const CDuiString& image_attr) {
    SetAttribute(_T("normalimage"), image_attr.GetData());
    SetAttribute(_T("hotimage"), image_attr.GetData());
    SetAttribute(_T("pushedimage"), image_attr.GetData());
}

GroupListUI::GroupListUI() {
    const Theme& theme = GetTheme();
    SetBkColor(theme.panel);
    SetAttribute(_T("bordercolor"), theme.list_border_color);
    SetAttribute(_T("bordersize"), theme.border_zero);
    SetAttribute(_T("inset"), theme.inset_zero);
    SetChildPadding(0);
}

ItemListUI::ItemListUI() {
    const Theme& theme = GetTheme();
    SetBkColor(theme.white);
    SetAttribute(_T("bordercolor"), theme.border_color);
    SetAttribute(_T("bordersize"), theme.border_zero);
    SetAttribute(_T("inset"), theme.inset_zero);
    SetChildPadding(0);
}

SearchBoxUI::SearchBoxUI() {
    const Theme& theme = GetTheme();
    SetFixedHeight(28);
    SetTextColor(theme.text);
    SetBkColor(theme.white);
    SetAttribute(_T("bordercolor"), theme.border_color);
    SetAttribute(_T("textpadding"), theme.search_padding);
}

TitleBarUI::TitleBarUI() {
    const Theme& theme = GetTheme();
    SetName(_T("top_bar"));
    SetFixedHeight(35);
    SetBkColor(theme.panel);
    SetAttribute(_T("childvalign"), _T("top"));
    SetAttribute(_T("childpadding"), _T("0"));
    SetAttribute(_T("inset"), theme.title_inset);
    SetAttribute(_T("bordercolor"), theme.border_color);
    SetAttribute(_T("bordersize"), theme.title_border_size);
}

} // namespace appui
