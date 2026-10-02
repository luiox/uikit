#include "uikit/duilib/toast.h"

#include <cmath>

#include "message_icon.h"
#include "uikit/duilib/compat.h"
#include "uikit/duilib/controls.h"

namespace uikit {

namespace {

using DuiLib::CDuiString;

// 与 messagebox.cpp 同款：tokens 的 family 存 UTF-8 窄串，UNICODE 构建要宽字符。
CDuiString Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return CDuiString();
    }
    const int need = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(),
                                         static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(need), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), &wide[0], need);
    return CDuiString(wide.c_str());
}

// 行数估算：CJK 全角字形按“字号 + 1”步进（与消息框同参数）。
int EstimateLineCount(const CDuiString& text, int text_width_px) {
    const int font_px = 12;
    const int cols_per_line = (text_width_px > 0) ? text_width_px / (font_px + 1) : 24;
    if (cols_per_line <= 0) {
        return 1;
    }
    const int len = static_cast<int>(_tcslen(text.GetData()));
    return (len > 0) ? (len + cols_per_line - 1) / cols_per_line : 1;
}

// 同屏已有 UIKitToast 可见窗计数 → 纵向错开位。
int CountVisibleToasts() {
    int count = 0;
    HWND hwnd = nullptr;
    while ((hwnd = ::FindWindowEx(nullptr, hwnd, _T("UIKitToast"), nullptr)) != nullptr) {
        if (::IsWindowVisible(hwnd)) {
            ++count;
        }
    }
    return count;
}

}  // namespace

Toast::Toast(const ToastSpec& spec) : spec_(spec) {
    sizebox_px_ = 0;     // 通知不可缩放
    caption_h_px_ = 0;   // 无拖拽区
}

DuiLib::CControlUI* Toast::BuildRootUi() {
    const ResolvedTheme& t = ActiveTheme();
    std::string family("Microsoft YaHei UI");
    for (const auto& f : t.fonts) {
        if (f.role == "default") {
            family = f.family;
            break;
        }
    }
    m_pm.AddFont(detail::kLevelGlyphFontId, Utf8ToWide(family).GetData(), 18, TRUE, FALSE, FALSE);

    auto* root = new duilib::PanelUI();
    root->OnThemeChanged();
    root->SetAttribute(_T("bordercolor"), duilib::attr(t.color.border));
    root->SetAttribute(_T("bordersize"), _T("1"));

    auto* body = new DuiLib::CHorizontalLayoutUI();
    CDuiString inset;
    inset.Format(_T("%d,%d,%d,%d"), t.metrics.window_inset, t.metrics.window_inset,
                 t.metrics.window_inset, t.metrics.window_inset);
    body->SetAttribute(_T("inset"), inset.GetData());
    body->SetAttribute(_T("childpadding"), _T("12"));
    body->SetAttribute(_T("childvalign"), _T("center"));
    body->Add(new detail::MessageIconUI(spec_.level));
    auto* text = new duilib::LabelUI();
    text->SetText(spec_.text);
    text->SetTextStyle(DT_LEFT | DT_WORDBREAK | DT_VCENTER);
    body->Add(text);
    root->Add(body);
    return root;
}

LRESULT Toast::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TIMER && wParam == 1) {
        CloseSelf();
        return 0;
    }
    return duilib::FramelessWindow::HandleMessage(uMsg, wParam, lParam);
}

void Toast::OnFinalMessage(HWND) {
    delete this;  // 自管理生命周期：Show 即发即忘
}

void Toast::CloseSelf() {
    if (closing_) {
        return;
    }
    closing_ = true;
    ::KillTimer(m_hWnd, 1);
    Close(0);
}

void Toast::Show(HWND owner, const ToastSpec& spec) {
    auto* toast = new Toast(spec);
    // NOACTIVATE：出现/消失都不抢焦点；TOOLWINDOW：不进任务栏/Alt-Tab。
    const DWORD ex_style = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST |
                           WS_EX_WINDOWEDGE;
    const DWORD style = WS_POPUP | WS_VISIBLE;
    HWND hwnd = toast->Create(owner, _T(""), style, ex_style);
    if (hwnd == nullptr) {
        delete toast;
        return;
    }

    // 尺寸：宽度按 spec，高度按文本行数（同消息框估算口径）。
    const ResolvedTheme& t = ActiveTheme();
    const int text_width = spec.width - t.metrics.window_inset * 2 - detail::kLevelIconSize - 12;
    const int lines = EstimateLineCount(spec.text, text_width);
    const int line_h = 20;
    const int content = (lines > 1) ? lines * line_h + 8 : detail::kLevelIconSize;
    const int height = t.metrics.window_inset * 2 + content;
    toast->ResizeClient(duilib::scale(&toast->paint_manager(), spec.width),
                        duilib::scale(&toast->paint_manager(), height));

    // 位置：owner 顶中下方（无 owner 落主屏工作区顶中）；多例向下错开。
    const int w = duilib::scale(&toast->paint_manager(), spec.width);
    const int h = duilib::scale(&toast->paint_manager(), height);
    RECT rc_owner = {0, 0, 0, 0};
    RECT rc_area = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    HMONITOR monitor = ::MonitorFromWindow(owner ? owner : hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};
    if (::GetMonitorInfo(monitor, &mi)) {
        rc_area = mi.rcWork;
    }
    if (owner != nullptr && ::IsWindow(owner)) {
        ::GetWindowRect(owner, &rc_owner);
    }
    int x = rc_area.left + ((rc_area.right - rc_area.left) - w) / 2;
    int y = rc_area.top + 48;
    if (rc_owner.right > rc_owner.left) {
        x = rc_owner.left + ((rc_owner.right - rc_owner.left) - w) / 2;
        y = rc_owner.top + 48;
    }
    y += CountVisibleToasts() * (h + 8);
    ::SetWindowPos(hwnd, HWND_TOPMOST, x, y, 0, 0,
                   SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

    if (spec.duration_ms > 0) {
        ::SetTimer(hwnd, 1, static_cast<UINT>(spec.duration_ms), nullptr);
    }
}

}  // namespace uikit
