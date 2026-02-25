#pragma once

#include <UIlib.h>

namespace appui {

struct Theme {
    DWORD text = 0xFF5A5A5A;
    DWORD panel = 0xFFE6E6E6;
    DWORD white = 0xFFFFFFFF;
    DWORD border = 0xFFD2D2D2;
    DWORD list_border = 0xFFCFD7E0;
    LPCTSTR border_color = _T("0xFFD2D2D2");
    LPCTSTR list_border_color = _T("0xFFCFD7E0");
    LPCTSTR icon_btn_normal = _T("0xFFF2F2F2");
    LPCTSTR icon_btn_hot = _T("0xFFE4E4E4");
    LPCTSTR icon_btn_pushed = _T("0xFFD7D7D7");
    LPCTSTR border_zero = _T("0");
    LPCTSTR border_one = _T("1");
    LPCTSTR inset_zero = _T("0,0,0,0");
    LPCTSTR title_inset = _T("12,0,0,0");
    LPCTSTR title_border_size = _T("0,0,0,0");
    LPCTSTR search_padding = _T("6,2,6,2");
};

const Theme& GetTheme();

class IconButtonUI : public DuiLib::CButtonUI {
public:
    IconButtonUI();
    LPCTSTR GetClass() const override { return _T("AppIconButton"); }
    void SetSvgImage(const DuiLib::CDuiString& image_attr);
};

class GroupListUI : public DuiLib::CListUI {
public:
    GroupListUI();
    LPCTSTR GetClass() const override { return _T("AppGroupList"); }
};

class ItemListUI : public DuiLib::CListUI {
public:
    ItemListUI();
    LPCTSTR GetClass() const override { return _T("AppItemList"); }
};

class SearchBoxUI : public DuiLib::CEditUI {
public:
    SearchBoxUI();
    LPCTSTR GetClass() const override { return _T("AppSearchBox"); }
};

class TitleBarUI : public DuiLib::CHorizontalLayoutUI {
public:
    TitleBarUI();
    LPCTSTR GetClass() const override { return _T("AppTitleBar"); }
};

} // namespace appui
