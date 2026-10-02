#include "uikit/duilib/frame_window.h"

#include "uikit/duilib/compat.h"
#include "uikit/duilib/controls.h"

namespace uikit {

using DuiLib::CDuiRect;
using DuiLib::CDuiSize;
using DuiLib::CDuiString;
using DuiLib::UIRender;

// 自绘窗口钮：底与字形全部主题化——悬停 icon_hot（close 悬停 danger + on_accent
// 字形，native 语义），常态透明；直角全高（贴 titlebar 全高，native 窗口钮
// 的满条悬停观感）。字形为 DrawLine/DrawRect 几何（无图片资产）。
class FrameWindow::CaptionButton : public duilib::ButtonUI {
public:
    enum class Glyph { Min, Max, Restore, Close };

    explicit CaptionButton(Glyph glyph) : glyph_(glyph) {
        const ResolvedTheme& t = ActiveTheme();
        SetFixedWidth(t.metrics.icon_button_size + 8);
        SetFixedHeight(t.metrics.titlebar_height);  // 满条高：悬停底贴齐标题栏
        SetAttribute(_T("bordercolor"), _T("0x00000000"));
        SetAttribute(_T("bordersize"), _T("0"));
    }

    void SetGlyph(Glyph glyph) {
        glyph_ = glyph;
        Invalidate();
    }

protected:
    void PaintStatusImage(UIRender* pRender) override {
        const ResolvedTheme& t = ActiveTheme();
        const bool hot = IsHotState() || IsPushedState();
        if (hot && IsEnabled()) {
            const Color bg = (glyph_ == Glyph::Close) ? t.color.danger : t.color.icon_hot;
            pRender->DrawColor(m_rcItem, CDuiSize(0, 0), bg);
        }

        const int cx = (m_rcItem.left + m_rcItem.right) / 2;
        const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
        const int r = duilib::scale(m_pManager, 5);    // 字形半径（96 基准）
        const int d = duilib::scale(m_pManager, 3);    // restore 双框错位
        const int stroke = duilib::scale(m_pManager, 1);
        Color glyph = t.color.text;
        if (!IsEnabled()) {
            glyph = t.control_disabled;
        } else if (hot && glyph_ == Glyph::Close) {
            glyph = t.color.on_accent;  // danger 底上转白，保持对比
        }
        switch (glyph_) {
            case Glyph::Min:
                // 停在下半（native 观感），一条横线。
                pRender->DrawLine(cx - r, cy + r / 2, cx + r, cy + r / 2, stroke, glyph);
                break;
            case Glyph::Max:
                pRender->DrawRect(CDuiRect{cx - r, cy - r, cx + r, cy + r}, stroke, glyph);
                break;
            case Glyph::Restore:
                // 后框只露上/右两缘，前框完整——避免交叉线发闷。
                pRender->DrawLine(cx - r + d, cy - r, cx + r, cy - r, stroke, glyph);
                pRender->DrawLine(cx + r, cy - r, cx + r, cy + r - d, stroke, glyph);
                pRender->DrawRect(CDuiRect{cx - r, cy - r + d, cx + r - d, cy + r}, stroke, glyph);
                break;
            case Glyph::Close:
                pRender->DrawLine(cx - r, cy - r, cx + r, cy + r, stroke, glyph);
                pRender->DrawLine(cx - r, cy + r, cx + r, cy - r, stroke, glyph);
                break;
        }
    }

private:
    Glyph glyph_;
};

void FrameWindow::SetTitleText(LPCTSTR text) {
    title_ = text;
}

void FrameWindow::ShowMinimizeButton(bool show) {
    show_min_ = show;
}

void FrameWindow::ShowMaximizeButton(bool show) {
    show_max_ = show;
}

DuiLib::CControlUI* FrameWindow::BuildRootUi() {
    auto* root = new duilib::PanelUI();
    root->OnThemeChanged();

    auto* top = new duilib::TitleBarUI();
    if (!title_.IsEmpty()) {
        auto* label = new duilib::LabelUI();
        label->SetText(title_);
        label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        top->Add(label);  // 不设固定宽：拉伸占满，把窗口钮推到右缘
    }
    min_button_ = new CaptionButton(CaptionButton::Glyph::Min);
    min_button_->SetName(_T("fw_min"));
    top->Add(min_button_);
    max_button_ = new CaptionButton(CaptionButton::Glyph::Max);
    max_button_->SetName(_T("fw_max"));
    top->Add(max_button_);
    auto* close = new CaptionButton(CaptionButton::Glyph::Close);
    close->SetName(_T("fw_close"));
    top->Add(close);
    if (!show_min_) {
        min_button_->SetVisible(false);
    }
    if (!show_max_) {
        max_button_->SetVisible(false);
    }
    zoomed_ = (m_hWnd != nullptr) && ::IsZoomed(m_hWnd) != 0;
    max_button_->SetGlyph(zoomed_ ? CaptionButton::Glyph::Restore : CaptionButton::Glyph::Max);
    root->Add(top);

    auto* content = new duilib::PanelUI();
    root->Add(content);
    InitContent(content);
    return root;
}


void FrameWindow::Notify(DuiLib::TNotifyUI& msg) {
    if (msg.sType == _T("click")) {
        const CDuiString& name = msg.pSender->GetName();
        if (name == _T("fw_min")) {
            ::ShowWindow(m_hWnd, SW_MINIMIZE);
            return;
        }
        if (name == _T("fw_max")) {
            ToggleMaximize();
            return;
        }
        if (name == _T("fw_close")) {
            Close();
            return;
        }
    }
    // 其余通知留给子类（INotifyUI::Notify 为纯虚，无基类实现可链）。
}

LRESULT FrameWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_SIZE) {
        UpdateMaxGlyph();
    }
    return duilib::FramelessWindow::HandleMessage(uMsg, wParam, lParam);
}

LRESULT FrameWindow::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    // 客户区 = 整窗的配方在最大化态会让内容越出工作区（Windows 给最大化窗
    // 补了被剥掉的框厚）：此处把客户区按框厚收回，与 native 最大化对齐。
    if (wParam != 0 && m_hWnd != nullptr && ::IsZoomed(m_hWnd)) {
        auto* rc = reinterpret_cast<RECT*>(lParam);  // rgrc[0]
        const int frame_x = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);
        const int frame_y = ::GetSystemMetrics(SM_CYSIZEFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);
        ::InflateRect(rc, -frame_x, -frame_y);
        bHandled = TRUE;
        return 0;
    }
    return duilib::FramelessWindow::OnNcCalcSize(uMsg, wParam, lParam, bHandled);
}

void FrameWindow::ToggleMaximize() {
    if (m_hWnd == nullptr) {
        return;
    }
    ::ShowWindow(m_hWnd, ::IsZoomed(m_hWnd) ? SW_RESTORE : SW_MAXIMIZE);
}

void FrameWindow::UpdateMaxGlyph() {
    if (max_button_ == nullptr || m_hWnd == nullptr) {
        return;
    }
    const bool zoomed = ::IsZoomed(m_hWnd) != 0;
    if (zoomed != zoomed_) {
        zoomed_ = zoomed;
        max_button_->SetGlyph(zoomed ? CaptionButton::Glyph::Restore
                                     : CaptionButton::Glyph::Max);
    }
}

}  // namespace uikit
