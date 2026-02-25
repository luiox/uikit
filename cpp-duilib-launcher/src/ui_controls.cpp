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
    // CListUI 默认带 Header，占位会造成顶部“空白条”，此处强制隐藏。
    if (GetHeader() != nullptr) {
        GetHeader()->SetVisible(false);
    }
    SetChildPadding(0);
}

ItemListUI::ItemListUI() {
    const Theme& theme = GetTheme();
    SetBkColor(theme.white);
    SetAttribute(_T("bordercolor"), theme.border_color);
    SetAttribute(_T("bordersize"), theme.border_zero);
    SetAttribute(_T("inset"), theme.inset_zero);
    // 搜索结果与启动项共用该列表，同样去掉默认 Header 占位。
    if (GetHeader() != nullptr) {
        GetHeader()->SetVisible(false);
    }
    SetChildPadding(0);
}

SearchBoxUI::SearchBoxUI() {
    const Theme& theme = GetTheme();
    SetFixedHeight(30);
    SetFont(1);
    SetTextColor(theme.text);
    SetBkColor(0xFFD2D2D2);
    SetAttribute(_T("bordercolor"), _T("0xFFD2D2D2"));
    SetAttribute(_T("bordersize"), _T("0"));
    SetAttribute(_T("nativebkcolor"), _T("0xFFD2D2D2"));
    SetAttribute(_T("textpadding"), _T("8,4,8,4"));
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
