#include "uikit/duilib/frameless.h"

#include "uikit/duilib/compat.h"

namespace uikit::duilib {

void StripCaption(HWND hwnd) {
    LONG style = ::GetWindowLong(hwnd, GWL_STYLE);
    style &= ~WS_CAPTION;
    ::SetWindowLong(hwnd, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
}

int FramelessWindow::DefaultFontPx() const {
    for (const auto& f : ActiveTheme().fonts) {
        if (f.role == "default") {
            return f.size_px;
        }
    }
    return 12;
}

LRESULT FramelessWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    StripCaption(m_hWnd);

    m_pm.Init(m_hWnd, GetWindowClassName(), this);
    InstallFonts(m_pm, ActiveTheme(), DefaultFontPx());

    DuiLib::CControlUI* root = BuildRootUi();
    if (root == nullptr) {
        bHandled = TRUE;
        return -1;
    }
    m_pm.AttachDialog(root);
    m_pm.AddNotifier(this);

    const RECT rc_caption{0, 0, 0, scale(&m_pm, caption_h_px_)};
    m_pm.SetCaptionRect(rc_caption);
    const int sb = scale(&m_pm, sizebox_px_);
    const RECT rc_sizebox{sb, sb, sb, sb};
    m_pm.SetSizeBox(rc_sizebox);

    bHandled = FALSE;
    return 0;
}

LRESULT FramelessWindow::OnNcCalcSize(UINT, WPARAM, LPARAM, BOOL&) {
    // 客户区 = 整个窗口（WindowImplBase 同款）；否则 DWM 在 WS_THICKFRAME 下
    // 会保留并绘制默认标题栏。
    return 0;
}

LRESULT FramelessWindow::OnNcPaint(UINT, WPARAM, LPARAM, BOOL&) {
    return 0;
}

namespace {

// 命中链上出现交互控件即放行点击；否则视为静态区（标题文字/空白）可拖拽。
bool ChainHasInteractive(DuiLib::CControlUI* control) {
    for (DuiLib::CControlUI* p = control; p != nullptr; p = p->GetParent()) {
        DuiLib::CDuiString cls = p->GetClass();
        cls.MakeLower();
        if (cls.Find(_T("button")) >= 0 || cls.Find(_T("edit")) >= 0 ||
            cls.Find(_T("list")) >= 0 || cls.Find(_T("combo")) >= 0 ||
            cls.Find(_T("scrollbar")) >= 0 || cls.Find(_T("slider")) >= 0) {
            return true;
        }
    }
    return false;
}

}  // namespace

LRESULT FramelessWindow::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    DuiLib::CDuiPoint pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ::ScreenToClient(m_hWnd, &pt);

    RECT rc_client{};
    ::GetClientRect(m_hWnd, &rc_client);
    if (!::IsZoomed(m_hWnd)) {
        const RECT& sb = m_pm.GetSizeBox();
        if (pt.y < rc_client.top + sb.top) {
            if (pt.x < rc_client.left + sb.left) return HTTOPLEFT;
            if (pt.x > rc_client.right - sb.right) return HTTOPRIGHT;
            return HTTOP;
        }
        if (pt.y > rc_client.bottom - sb.bottom) {
            if (pt.x < rc_client.left + sb.left) return HTBOTTOMLEFT;
            if (pt.x > rc_client.right - sb.right) return HTBOTTOMRIGHT;
            return HTBOTTOM;
        }
        if (pt.x < rc_client.left + sb.left) return HTLEFT;
        if (pt.x > rc_client.right - sb.right) return HTRIGHT;
    }

    RECT rc_caption = m_pm.GetCaptionRect();
    if (auto* dpi = m_pm.GetDPIObj()) {
        rc_caption = dpi->ScaleRect(rc_caption);
    }
    if (pt.x >= rc_caption.left && pt.x < rc_client.right - rc_caption.right &&
        pt.y >= rc_caption.top && pt.y < rc_caption.bottom) {
        if (!ChainHasInteractive(m_pm.FindControl(pt))) {
            return HTCAPTION;
        }
    }
    return HTCLIENT;
}

int FramelessWindow::Run(LPCTSTR title, int width, int height, int caption_h_px, int sizebox_px) {
    caption_h_px_ = caption_h_px;
    sizebox_px_ = sizebox_px;

    HWND hwnd = Create(nullptr, title, UI_WNDSTYLE_FRAME | WS_SIZEBOX, WS_EX_WINDOWEDGE);
    if (hwnd == nullptr) {
        return 2;
    }
    // 逻辑 → 物理：m_pm 已在 WM_CREATE 里初始化，可安全取 DPI。
    ResizeClient(scale(&m_pm, width), scale(&m_pm, height));
    CenterWindow();
    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);

    DuiLib::DuiLibPaintManagerUI::MessageLoop();
    return 0;
}

}  // namespace uikit::duilib
