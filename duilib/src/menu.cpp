#include "uikit/duilib/menu.h"

#include <algorithm>

#include "popup_surface.h"
#include "uikit/duilib/compat.h"
#include "uikit/duilib/controls.h"

namespace uikit {

namespace {

using DuiLib::CDuiPoint;
using DuiLib::CDuiSize;
using DuiLib::CDuiString;
using DuiLib::CControlUI;
using DuiLib::UIRender;

using uikit::duilib::ButtonUI;
using uikit::duilib::scale;

}  // namespace

// 菜单条目行：常态透明，悬停/按下/键盘高亮 surface_hover；禁用项文字退色
// 且不响应。颜色每帧直读 ActiveTheme()（弹层毫秒级生命周期，切主题瞬态无感）。
class Menu::Row : public ButtonUI {
public:
    Row(Menu* owner, const MenuItem& item) : owner_(owner), id_(item.id) {
        SetText(item.text);
        const ResolvedTheme& t = ActiveTheme();
        SetFixedHeight(t.metrics.control_height);
        // 文本左对齐 + 控件水平内边距（条目列的呼吸感）。
        CDuiString padding;
        padding.Format(_T("%d,0,%d,0"), t.metrics.control_hpad, t.metrics.control_hpad);
        SetAttribute(_T("textpadding"), padding.GetData());
        SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        if (!item.enabled) {
            SetEnabled(false);
        }
    }

protected:
    bool Activate() override {
        if (!ButtonUI::Activate()) {
            return false;
        }
        owner_->Pick(id_);
        return true;
    }

    // 悬停/按下/键盘高亮（SetHotState 置同一 UISTATE_HOT 位）都画 surface_hover。
    void PaintStatusImage(UIRender* pRender) override {
        const ResolvedTheme& t = ActiveTheme();
        if (IsEnabled() && (IsHotState() || IsPushedState())) {
            const int r = scale(m_pManager, t.metrics.radius_control);
            pRender->DrawColor(m_rcItem, CDuiSize(r, r), t.color.surface_hover);
        }
    }

    void PaintText(UIRender* pRender) override {
        const ResolvedTheme& t = ActiveTheme();
        CButtonUI::SetTextColor(IsEnabled() ? t.color.text : t.control_disabled);
        CButtonUI::PaintText(pRender);
    }

private:
    Menu* owner_;
    int id_;
};

Menu::Menu(const std::vector<MenuItem>& items) : items_(items) {
    // 弹层不可缩放/无拖拽区（OnNcHitTest 的 sizebox 由 0 值矩形自然失效）。
    sizebox_px_ = 0;
    caption_h_px_ = 0;
}

DuiLib::CControlUI* Menu::BuildRootUi() {
    auto* root = new uikit::detail::PopupSurfaceUI();
    const ResolvedTheme& t = ActiveTheme();
    CDuiString inset;
    inset.Format(_T("%d,%d,%d,%d"), t.metrics.window_inset, t.metrics.window_inset,
                 t.metrics.window_inset, t.metrics.window_inset);
    root->SetAttribute(_T("inset"), inset.GetData());

    for (const MenuItem& item : items_) {
        auto* row = new Row(this, item);
        rows_.push_back(row);
        root->Add(row);
        if (item.separator_after) {
            auto* line = new CControlUI();
            line->SetFixedHeight(1);  // divider 走令牌色，行高 1px（主题密度内不做呼吸边距）
            line->SetBkColor(t.color.divider);
            root->Add(line);
        }
    }
    return root;
}

int Menu::MeasureMaxTextWidth() {
    // 实测条目文本宽（默认字体 HFONT + GetTextExtentPoint32）：比 toast 的
    // 行数估算口径准，代价仅一次内存 DC 往返，决定弹层宽度用。
    HFONT font = nullptr;
    if (auto* font_info = m_pm.GetDefaultFontInfo()) {
        font = font_info->GetHFONT(&m_pm);
    }
    if (font == nullptr) {
        return 0;
    }
    HDC dc = ::CreateCompatibleDC(nullptr);
    HGDIOBJ old = ::SelectObject(dc, font);
    int max_cx = 0;
    for (const MenuItem& item : items_) {
        const LPCTSTR text = item.text.GetData();
        SIZE sz = {0, 0};
        if (::GetTextExtentPoint32(dc, text, static_cast<int>(_tcslen(text)), &sz)) {
            max_cx = (sz.cx > max_cx) ? sz.cx : max_cx;
        }
    }
    ::SelectObject(dc, old);
    ::DeleteDC(dc);
    return max_cx;
}

void Menu::Pick(int id) {
    if (closed_) {
        return;
    }
    closed_ = true;
    Close(static_cast<UINT>(id));  // Close 投递 WM_CLOSE，wParam 即选择结果
}

void Menu::Cancel() {
    if (closed_) {
        return;
    }
    closed_ = true;
    Close(static_cast<UINT>(-1));
}

void Menu::MoveHighlight(int delta) {
    if (rows_.empty()) {
        return;
    }
    const int count = static_cast<int>(rows_.size());
    int next = kb_index_;
    for (int step = 0; step < count; ++step) {
        next += delta;
        if (next < 0) {
            next = count - 1;
        }
        if (next >= count) {
            next = 0;
        }
        if (rows_[next]->IsEnabled()) {
            break;  // 环绕一周找启用项
        }
    }
    if (!rows_[next]->IsEnabled()) {
        return;  // 全部禁用：原地不动
    }
    if (kb_index_ >= 0 && kb_index_ < count) {
        rows_[kb_index_]->SetHotState(false);
    }
    kb_index_ = next;
    rows_[kb_index_]->SetHotState(true);
}

LRESULT Menu::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN:
            // 弹层持有焦点（Show 里显式抢前台），键盘语义在此收口。
            if (wParam == VK_ESCAPE) {
                Cancel();
                return 0;
            }
            if (wParam == VK_RETURN && kb_index_ >= 0 &&
                kb_index_ < static_cast<int>(rows_.size()) && rows_[kb_index_]->IsEnabled()) {
                Pick(items_[kb_index_].id);
                return 0;
            }
            if (wParam == VK_UP || wParam == VK_DOWN) {
                MoveHighlight(wParam == VK_UP ? -1 : 1);
                return 0;
            }
            break;
        case WM_KILLFOCUS:
            // 点击其他应用/窗口抢走焦点（捕获只覆盖鼠标，键盘焦点丢了也该关）。
            Cancel();
            break;
        case WM_CAPTURECHANGED:
            // 捕获被第三方窗口抢走（罕见）：视同失焦关闭。
            Cancel();
            break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            // SetCapture 后所有鼠标按下都进弹层：按下点在窗外 = 点击外部 → 关闭。
            // 只拦「按下」不拦「抬起」：弹层刚弹出时回放的旧 WM_LBUTTONUP 不会
            // 被误判成外部点击。
            const CDuiPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            RECT rc = {0, 0, 0, 0};
            ::GetWindowRect(m_hWnd, &rc);  // 客户区 = 窗口矩形（OnNcCalcSize 返回 0）
            if (pt.x < 0 || pt.y < 0 || pt.x >= rc.right - rc.left ||
                pt.y >= rc.bottom - rc.top) {
                Cancel();
                return 0;
            }
            break;
        }
        default:
            break;
    }
    return duilib::FramelessWindow::HandleMessage(uMsg, wParam, lParam);
}

int Menu::Show(HWND owner, const POINT& screen_pt, const std::vector<MenuItem>& items) {
    if (items.empty()) {
        return -1;
    }
    auto* menu = new Menu(items);
    // 有任务栏豁免（TOOLWINDOW）+ 压顶（TOPMOST）；不加 NOACTIVATE——菜单要
    // 收键盘，必须能成为前台（fork 菜单同款 SetForegroundWindow 配方）。
    const DWORD ex_style = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_WINDOWEDGE;
    HWND hwnd = menu->Create(owner, _T(""), WS_POPUP, ex_style);
    if (hwnd == nullptr) {
        delete menu;
        return -1;
    }

    // 尺寸：宽度 = 实测最长条目 + 行内边距 + 弹层内边距，最小 160 逻辑像素；
    // 高度 = 条目行 × control_height + 分隔线 + 上下 window_inset。
    const ResolvedTheme& t = ActiveTheme();
    DuiLib::CPaintManagerUI& pm = menu->paint_manager();
    const int max_cx = menu->MeasureMaxTextWidth();
    const int width = (std::max)(scale(&pm, 160),
                                 max_cx + scale(&pm, t.metrics.control_hpad * 2 +
                                                      t.metrics.window_inset * 2));
    int rows_logical = 0;
    for (const MenuItem& item : items) {
        rows_logical += t.metrics.control_height;
        if (item.separator_after) {
            rows_logical += 1;
        }
    }
    const int height = scale(&pm, t.metrics.window_inset * 2 + rows_logical);
    menu->ResizeClient(width, height);

    // 位置：贴 anchor 点展开，越出屏幕工作区则向内收（下方放不下翻到上方）。
    RECT rc_work = {screen_pt.x, screen_pt.y, screen_pt.x + 1, screen_pt.y + 1};
    if (const HMONITOR monitor = ::MonitorFromPoint(screen_pt, MONITOR_DEFAULTTONEAREST)) {
        MONITORINFO mi = {sizeof(mi)};
        if (::GetMonitorInfo(monitor, &mi)) {
            rc_work = mi.rcWork;
        }
    }
    int x = screen_pt.x;
    int y = screen_pt.y;
    if (x + width > rc_work.right) {
        x = rc_work.right - width;
    }
    if (x < rc_work.left) {
        x = rc_work.left;
    }
    if (y + height > rc_work.bottom) {
        y = screen_pt.y - height;  // 向上翻转（anchor 点为下缘）
    }
    if (y < rc_work.top) {
        y = rc_work.top;
    }
    ::SetWindowPos(hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);

    // —— 模态段（fork ShowModal 口径的展开版，为在显示后、进泵前插入
    // 抢前台/焦点/捕获三步）——
    ::ShowWindow(hwnd, SW_SHOWNORMAL);
    if (owner != nullptr && ::IsWindow(owner)) {
        ::EnableWindow(owner, FALSE);
    }
    ::SetForegroundWindow(hwnd);
    ::SetFocus(hwnd);
    // 捕获鼠标：owner 已禁用（点它不动焦点 → KILLFOCUS 不触发），外点关闭
    // 靠捕获路径兜底（见 HandleMessage 的 WM_LBUTTONDOWN 分支）。
    ::SetCapture(hwnd);

    int result = -1;
    MSG msg = {0};
    while (::IsWindow(hwnd) && ::GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_CLOSE && msg.hwnd == hwnd) {
            result = static_cast<int>(msg.wParam);
        }
        if (!DuiLib::CPaintManagerUI::TranslateMessage(&msg)) {
            ::TranslateMessage(&msg);
        }
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            break;
        }
    }
    if (msg.message == WM_QUIT) {
        ::PostQuitMessage(static_cast<int>(msg.wParam));  // 吞掉了就补投（ShowModal 同款）
    }
    if (owner != nullptr && ::IsWindow(owner)) {
        ::EnableWindow(owner, TRUE);
        ::SetFocus(owner);  // 焦点归还宿主（messagebox 同款）
    }
    ::DestroyWindow(hwnd);  // 循环经 WM_CLOSE 自毁后的兜底（已毁则空操作）
    delete menu;            // Show 自管理生命周期，调用方无句柄可持
    return result;
}

}  // namespace uikit
