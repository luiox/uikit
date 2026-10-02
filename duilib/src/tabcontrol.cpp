#include "uikit/duilib/tabcontrol.h"

namespace uikit {

using DuiLib::CDuiSize;
using DuiLib::CControlUI;
using DuiLib::UIRender;

namespace {

// 页签最小宽（96 基准逻辑像素）：超短标题也保证可点、可读。
constexpr int kMinTabWidth = 64;
// 选中下划线高（96 基准逻辑像素）。
constexpr int kUnderlineHeight = 3;

}  // namespace

// 内部页签钮：点击不经窗口 Notify 接线，Activate 直接回调宿主切页（同
// SpinBox::StepButton 的自持模式）。宽度走 EstimateSize 通道——duilib 横向
// 布局对未设固定宽的子控件以 EstimateSize 结果定宽，而度量与绘制同源
// （manager 字体，RebuildFont 后随 DPI 缩放），因此无需另备 GDI 测量。
class TabControl::TabButton : public duilib::ButtonUI {
public:
    TabButton(TabControl* owner, int index, LPCTSTR title)
        : owner_(owner), index_(index) {
        SetText(title);
        // 视觉完全自绘：透明底（ButtonUI 预设）、显式关描边。
        SetStateColors(color::kTransparent, color::kTransparent, color::kTransparent);
        SetAttribute(_T("bordercolor"), _T("0x00000000"));
        SetAttribute(_T("bordersize"), _T("0"));
        SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    void SetSelected(bool selected) {
        if (selected_ == selected) {
            return;
        }
        selected_ = selected;
        Invalidate();
    }

protected:
    bool Activate() override {
        if (!ButtonUI::Activate()) {
            return false;
        }
        owner_->SetCurSel(index_);
        return true;
    }

    // 布局定宽：文本实测宽 + 左右各 control_hpad，下限 64 逻辑像素（均按
    // DPI 缩放）。度量按 DPI 档位缓存，避免每次排版都进文本引擎。
    // 高度返回 0：横向布局对 0 高子控件拉伸到整行（页签头全高命中区）。
    CDuiSize EstimateSize(CDuiSize sz_available) override {
        (void)sz_available;
        const ResolvedTheme& t = ActiveTheme();
        const int pad = duilib::scale(m_pManager, t.metrics.control_hpad) * 2;
        int width = MeasureTextPx();
        if (width > 0) {
            width += pad;
        }
        const int min_width = duilib::scale(m_pManager, kMinTabWidth);
        return CDuiSize(width > min_width ? width : min_width, 0);
    }

    void PaintText(UIRender* pRender) override {
        // 文本色每帧从主题解析：选中/悬停 = 正文，未选中 = 弱化，禁用退色。
        const ResolvedTheme& t = ActiveTheme();
        Color text_color = t.text_secondary;
        if (!IsEnabled()) {
            text_color = t.text_disabled;
        } else if (selected_ || IsHotState() || IsPushedState()) {
            text_color = t.color.text;
        }
        ButtonUI::SetTextColor(text_color);
        ButtonUI::PaintText(pRender);
    }

    void PaintStatusImage(UIRender* pRender) override {
        ButtonUI::PaintStatusImage(pRender);  // 透明底，无填充
        if (!selected_) {
            return;
        }
        // 选中下划线：整页签宽、3 逻辑像素高，贴控件底缘（控件即页签头全高）。
        const ResolvedTheme& t = ActiveTheme();
        const int h = duilib::scale(m_pManager, kUnderlineHeight);
        const DuiLib::CDuiRect line{m_rcItem.left, m_rcItem.bottom - h, m_rcItem.right,
                                    m_rcItem.bottom};
        pRender->DrawColor(line, CDuiSize(0, 0), t.color.accent);
    }

private:
    // 文本实测宽（物理像素，含 DPI）。manager 未挂（异常时序）返回 0，
    // EstimateSize 退到最小宽。
    int MeasureTextPx() {
        auto* manager = GetManager();
        if (manager == nullptr) {
            return 0;
        }
        auto* dpi = manager->GetDPIObj();
        const int scale_now = (dpi != nullptr) ? dpi->GetScale() : 96;
        // 缓存按 DPI 档位失效（SetDPI 会 RebuildFont，度量随之变化）。
        if (measured_dpi_ != scale_now || measured_cx_ <= 0) {
            measured_cx_ = 0;
            if (auto* render = manager->Render()) {
                measured_cx_ = render
                                   ->GetTextSize(GetText().GetData(), -1,
                                                 DT_SINGLELINE | DT_NOPREFIX)
                                   .cx;
            }
            measured_dpi_ = scale_now;
        }
        return measured_cx_;
    }

    TabControl* owner_;
    int index_;
    bool selected_ = false;
    int measured_cx_ = 0;
    int measured_dpi_ = 0;
};

TabControl::TabControl() {
    const ResolvedTheme& t = ActiveTheme();
    header_ = new DuiLib::CHorizontalLayoutUI();
    header_->SetFixedHeight(t.metrics.control_height);
    Add(header_);
    // 内容区：包内 CTabLayoutUI 自带"首个可见页为当前页、其余 Add 时自动
    // 隐藏、SelectItem 切可见性"的完整语义，本控件只做页签头联动，不重造切页。
    pages_ = new DuiLib::CTabLayoutUI();
    Add(pages_);
    OnThemeChanged();
}

void TabControl::OnThemeChanged() {
    // 页签头 panel 底；内容区不设底色（页面自备，透传宿主分区底）。
    header_->SetBkColor(ActiveTheme().color.panel);
}

void TabControl::AddPage(CControlUI* page, LPCTSTR title) {
    if (page == nullptr) {
        return;
    }
    auto* tab = new TabButton(this, static_cast<int>(tabs_.size()), title);
    tabs_.push_back(tab);
    header_->Add(tab);
    pages_->Add(page);
    if (cur_sel_ < 0) {
        // CTabLayoutUI::Add 让首个可见页成为当前页，页签头同步高亮。
        cur_sel_ = 0;
        tab->SetSelected(true);
    }
}

void TabControl::SetCurSel(int index) {
    if (index < 0 || index >= static_cast<int>(tabs_.size()) || index == cur_sel_) {
        return;
    }
    cur_sel_ = index;
    pages_->SelectItem(index);  // 内容区联动（内部切可见性并移动焦点）
    UpdateButtons();
    if (GetManager() != nullptr) {
        GetManager()->SendNotify(this, _T("tabchange"), static_cast<WPARAM>(index), 0);
    }
}

void TabControl::UpdateButtons() {
    for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
        tabs_[i]->SetSelected(i == cur_sel_);
    }
}

}  // namespace uikit
