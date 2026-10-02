#include "uikit/duilib/tooltip.h"

#include "uikit/duilib/compat.h"
#include "uikit/duilib/controls.h"

namespace uikit {

namespace {

using DuiLib::CDuiString;

// 气泡最大宽（96 基准逻辑像素），超出折行。
constexpr int kMaxWidth = 240;
// 弹出点与 target 边缘的间距（逻辑像素）。
constexpr int kGap = 8;
// 行高与折行富余：与 toast.cpp/messagebox.cpp 同口径，避免同屏两套行距。
constexpr int kLineHeight = 20;
constexpr int kExtraForWrap = 8;

// 行数估算：CJK 全角字形按"字号 + 1"步进（与消息框/悬浮通知完全同参）。
int EstimateLineCount(const CDuiString& text, int text_width_px) {
    const int font_px = 12;
    const int cols_per_line = (text_width_px > 0) ? text_width_px / (font_px + 1) : 24;
    if (cols_per_line <= 0) {
        return 1;
    }
    const int len = static_cast<int>(_tcslen(text.GetData()));
    return (len > 0) ? (len + cols_per_line - 1) / cols_per_line : 1;
}

// 气泡根容器：panel 底 + 圆角随 radius_control 令牌 + 1px 描边，绘制期直读
// ActiveTheme()。不用 PanelUI——它的底色/描边走属性链（直角），这里要随
// 令牌出圆角，须接管底色与描边的绘制。
class TipRoot : public DuiLib::CVerticalLayoutUI, public duilib::ThemeableControl {
public:
    LPCTSTR GetClass() const override { return _T("UIKitTooltipRoot"); }

protected:
    void PaintBkColor(DuiLib::UIRender* pRender) override {
        const ResolvedTheme& t = ActiveTheme();
        const int radius = duilib::scale(m_pManager, t.metrics.radius_control);
        pRender->DrawColor(m_rcItem, DuiLib::CDuiSize(radius, radius), t.color.panel);
    }

    void PaintBorder(DuiLib::UIRender* pRender) override {
        const ResolvedTheme& t = ActiveTheme();
        const int radius = duilib::scale(m_pManager, t.metrics.radius_control);
        if (radius > 0) {
            pRender->DrawRoundRect(m_rcItem, 1, DuiLib::CDuiSize(radius, radius),
                                   t.color.border);
        } else {
            pRender->DrawRect(m_rcItem, 1, t.color.border);
        }
    }
};

}  // namespace

Tooltip::Tooltip(const CDuiString& text, int duration_ms)
    : text_(text), duration_ms_(duration_ms) {
    sizebox_px_ = 0;     // 气泡不可缩放
    caption_h_px_ = 0;   // 无拖拽区
}

DuiLib::CControlUI* Tooltip::BuildRootUi() {
    auto* root = new TipRoot();
    auto* label = new duilib::LabelUI();
    label->SetText(text_.GetData());
    // 折行由 DT_WORDBREAK 承接；文本色在 LabelUI::PaintText 里实时取主题。
    label->SetTextStyle(DT_LEFT | DT_WORDBREAK | DT_VCENTER);
    const ResolvedTheme& t = ActiveTheme();
    CDuiString inset;
    inset.Format(_T("%d,%d,%d,%d"), t.metrics.window_inset, t.metrics.window_inset,
                 t.metrics.window_inset, t.metrics.window_inset);
    label->SetAttribute(_T("inset"), inset.GetData());
    root->Add(label);
    return root;
}

LRESULT Tooltip::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TIMER && wParam == 1) {
        CloseSelf();
        return 0;
    }
    return duilib::FramelessWindow::HandleMessage(uMsg, wParam, lParam);
}

void Tooltip::OnFinalMessage(HWND) {
    delete this;  // 自管理生命周期：Show 即发即忘
}

void Tooltip::CloseSelf() {
    if (closing_) {
        return;
    }
    closing_ = true;
    ::KillTimer(m_hWnd, 1);
    Close(0);
}

void Tooltip::Show(HWND owner, DuiLib::CControlUI* target, const CDuiString& text,
                   int duration_ms) {
    auto* manager = (target != nullptr) ? target->GetManager() : nullptr;
    if (manager == nullptr || text.IsEmpty()) {
        return;
    }
    const DuiLib::CDuiRect& rc_target = target->GetPos();
    if (rc_target.GetWidth() <= 0 && rc_target.GetHeight() <= 0) {
        return;  // 尚未排版：锚点无意义
    }
    if (owner == nullptr) {
        owner = manager->GetPaintWindow();
    }

    auto* tip = new Tooltip(text, duration_ms > 0 ? duration_ms : 3000);
    // NOACTIVATE：出现/消失都不抢焦点；TOOLWINDOW：不进任务栏/Alt-Tab。
    const DWORD ex_style = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST |
                           WS_EX_WINDOWEDGE;
    const DWORD style = WS_POPUP | WS_VISIBLE;
    HWND hwnd = tip->Create(owner, _T(""), style, ex_style);
    if (hwnd == nullptr) {
        delete tip;
        return;
    }

    // 尺寸：文本按本窗 manager 字体实测（物理像素）折回逻辑值定宽；超出
    // 240 上限按估算行数折行。全部逻辑值算完最后一次 scale，与 toast 相同。
    auto& pm = tip->paint_manager();
    const ResolvedTheme& t = ActiveTheme();
    auto* dpi = pm.GetDPIObj();
    const int dpi_scale = (dpi != nullptr) ? dpi->GetScale() : 96;
    const auto to_logical = [dpi_scale](int phys) {
        return (phys * 96 + dpi_scale - 1) / dpi_scale;
    };
    int text_logical = 0;
    if (auto* render = pm.Render()) {
        text_logical =
            to_logical(render->GetTextSize(text.GetData(), -1, DT_SINGLELINE | DT_NOPREFIX).cx);
    }
    // 内宽扣掉左右内边距与两侧 1px 描边；超宽折行后收至上限。
    const int inner = kMaxWidth - t.metrics.window_inset * 2 - 2;
    int lines = 1;
    if (text_logical > inner && text_logical > 0) {
        lines = EstimateLineCount(text, inner);
        text_logical = inner;
    }
    const int width = t.metrics.window_inset * 2 + text_logical + 2;  // +2 = 两侧描边
    const int content = (lines > 1) ? lines * kLineHeight + kExtraForWrap : kLineHeight;
    const int height = t.metrics.window_inset * 2 + content;

    const int w = duilib::scale(&pm, width);
    const int h = duilib::scale(&pm, height);
    tip->ResizeClient(w, h);

    // 圆角非 0 的主题：窗体本身是矩形，用窗区域裁出圆角（rgn 归系统所有，
    // 不用 DeleteObject）；直角主题跳过，省一次系统调用。
    const int radius = duilib::scale(&pm, t.metrics.radius_control);
    if (radius > 0) {
        ::SetWindowRgn(hwnd, ::CreateRoundRectRgn(0, 0, w + 1, h + 1, radius, radius), TRUE);
    }

    // 位置：target->GetPos() 是宿主客户区坐标，经宿主窗口换屏幕坐标
    // （frameless 宿主的 GetPaintWindow 即宿主窗体）。
    POINT pt_below{rc_target.left, rc_target.bottom};
    POINT pt_above{rc_target.left, rc_target.top};
    ::ClientToScreen(manager->GetPaintWindow(), &pt_below);
    ::ClientToScreen(manager->GetPaintWindow(), &pt_above);

    RECT rc_area{0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN)};
    HMONITOR monitor = ::MonitorFromPoint(pt_below, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};
    if (::GetMonitorInfo(monitor, &mi)) {
        rc_area = mi.rcWork;
    }
    const int gap = duilib::scale(&pm, kGap);
    int x = pt_below.x;
    if (x + w > rc_area.right) {
        x = rc_area.right - w;
    }
    if (x < rc_area.left) {
        x = rc_area.left;
    }
    int y = pt_below.y + gap;
    if (y + h > rc_area.bottom) {
        // 下方放不下：翻转到 target 上方（贴 target 顶边再留 gap）；
        // 仍溢出则压回工作区顶，保证气泡完整可见。
        y = pt_above.y - h - gap;
        if (y < rc_area.top) {
            y = rc_area.top;
        }
    }
    ::SetWindowPos(hwnd, HWND_TOPMOST, x, y, 0, 0,
                   SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

    ::SetTimer(hwnd, 1, static_cast<UINT>(tip->duration_ms_), nullptr);
}

}  // namespace uikit
