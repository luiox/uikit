#include "uikit/duilib/messagebox.h"

#include <cmath>

#include "message_icon.h"
#include "uikit/duilib/compat.h"
#include "uikit/duilib/controls.h"

namespace uikit {

using DuiLib::CDuiRect;
using DuiLib::CDuiSize;
using DuiLib::CDuiString;
using DuiLib::CControlUI;
using DuiLib::UIRender;

using uikit::duilib::ButtonUI;
using uikit::duilib::LabelUI;
using uikit::duilib::PanelUI;
using uikit::duilib::TitleBarUI;
using uikit::duilib::attr;
using uikit::duilib::scale;

// tokens 的 family 存 UTF-8 窄串；duilib UNICODE 构建要宽字符（applier 同款）。
namespace {

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

// 文本行数估算：CJK 为主的全角字形按“字号 + 1”步进，列宽去图标区与内边距。
int EstimateLineCount(const CDuiString& text, int text_width_px) {
    const int font_px = 12;  // 与 DefaultFontPx 主题默认一致
    const int cols_per_line = (text_width_px > 0) ? text_width_px / (font_px + 1) : 24;
    if (cols_per_line <= 0) {
        return 1;
    }
    const int len = static_cast<int>(_tcslen(text.GetData()));
    return (len > 0) ? (len + cols_per_line - 1) / cols_per_line : 1;
}

}  // namespace

MessageBox::MessageBox(const MessageBoxSpec& spec) : spec_(spec) {
    // 消息框不可缩放/最大化（OnNcHitTest 的 sizebox 由 0 值矩形自然失效）。
    sizebox_px_ = 0;
}

void MessageBox::BuildButtonDefs(std::vector<ButtonDef>& out) {
    using B = MessageButtons;
    using R = MessageResult;
    switch (spec_.buttons) {
        case B::Ok:
            out = {{R::Ok, _T("确定"), true}};
            default_result_ = R::Ok;
            cancel_result_ = R::Ok;
            break;
        case B::OkCancel:
            out = {{R::Ok, _T("确定"), true}, {R::Cancel, _T("取消"), false}};
            default_result_ = R::Ok;
            cancel_result_ = R::Cancel;
            break;
        case B::YesNo:
            out = {{R::Yes, _T("是"), true}, {R::No, _T("否"), false}};
            default_result_ = R::Yes;
            cancel_result_ = R::No;
            break;
        case B::YesNoCancel:
            out = {{R::Yes, _T("是"), true}, {R::No, _T("否"), false}, {R::Cancel, _T("取消"), false}};
            default_result_ = R::Yes;
            cancel_result_ = R::Cancel;
            break;
        case B::RetryCancel:
            out = {{R::Retry, _T("重试"), true}, {R::Cancel, _T("取消"), false}};
            default_result_ = R::Retry;
            cancel_result_ = R::Cancel;
            break;
        case B::AbortRetryIgnore:
            // 中止不做默认项（Enter 误触风险）；无取消项，Esc/X 仅关闭。
            out = {{R::Abort, _T("中止"), false}, {R::Retry, _T("重试"), true}, {R::Ignore, _T("忽略"), false}};
            default_result_ = R::Retry;
            cancel_result_ = R::None;
            break;
    }
}

DuiLib::CControlUI* MessageBox::BuildRootUi() {
    const ResolvedTheme& t = ActiveTheme();
    BuildButtonDefs(buttons_);

    // 级别字形用主题字体族加大加粗注册到本 manager。
    std::string family("Microsoft YaHei UI");
    for (const auto& f : t.fonts) {
        if (f.role == "default") {
            family = f.family;
            break;
        }
    }
    m_pm.AddFont(detail::kLevelGlyphFontId, Utf8ToWide(family).GetData(), 18, TRUE, FALSE, FALSE);

    auto* root = new PanelUI();
    root->OnThemeChanged();

    auto* top = new TitleBarUI();
    auto* title = new LabelUI();
    title->SetText(spec_.title);
    title->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    top->Add(title);
    auto* closer = new ButtonUI();
    closer->SetName(_T("mb_close"));
    closer->SetText(_T("✕"));
    closer->SetFixedWidth(t.metrics.icon_button_size);
    closer->SetFixedHeight(t.metrics.icon_button_size);
    top->Add(closer);
    root->Add(top);

    auto* body = new DuiLib::CHorizontalLayoutUI();
    CDuiString body_inset;
    body_inset.Format(_T("%d,%d,%d,0"), t.metrics.window_inset, t.metrics.window_inset,
                      t.metrics.window_inset);
    body->SetAttribute(_T("inset"), body_inset.GetData());
    body->SetAttribute(_T("childpadding"), _T("12"));
    body->SetAttribute(_T("childvalign"), _T("center"));
    body->Add(new detail::MessageIconUI(spec_.level));
    auto* text = new LabelUI();
    text->SetText(spec_.text);
    text->SetTextStyle(DT_LEFT | DT_WORDBREAK);
    body->Add(text);
    root->Add(body);

    auto* button_row = new DuiLib::CHorizontalLayoutUI();
    // 行内含上下 inset，按钮自身 control_height——固定高需把 inset 一并算入。
    button_row->SetFixedHeight(t.metrics.control_height + t.metrics.window_inset * 2);
    CDuiString row_inset;
    row_inset.Format(_T("%d,%d,%d,%d"), t.metrics.window_inset, t.metrics.window_inset,
                     t.metrics.window_inset, t.metrics.window_inset);
    button_row->SetAttribute(_T("inset"), row_inset.GetData());
    button_row->SetAttribute(_T("childpadding"), _T("8"));
    BuildButtonRow(button_row, buttons_);
    root->Add(button_row);

    return root;
}

void MessageBox::BuildButtonRow(DuiLib::CContainerUI* row, const std::vector<ButtonDef>& defs) {
    // 右对齐：先放拉伸占位，再依次挂按钮。
    row->Add(new DuiLib::CControlUI());
    for (const ButtonDef& def : defs) {
        auto* button = new ButtonUI();
        CDuiString name;
        name.Format(_T("mb_btn_%d"), static_cast<int>(def.result));
        button->SetName(name.GetData());
        button->SetText(def.text);
        button->SetFixedWidth(84);
        if (def.primary) {
            button->StylePrimary();
        } else {
            button->StyleSecondary();
        }
        row->Add(button);
    }
}

int MessageBox::EstimateContentHeight() {
    const ResolvedTheme& t = ActiveTheme();
    // 文本列宽 = 窗宽 - 两侧 inset - 图标 - 间距。
    const int text_width =
        scale(&m_pm, spec_.width - t.metrics.window_inset * 2 - detail::kLevelIconSize - 12);
    const int lines = EstimateLineCount(spec_.text, text_width);
    const int line_h = 20;  // 12px 字号的舒适行高
    const int body_h = (lines > 1) ? lines * line_h + 8 : (detail::kLevelIconSize > t.metrics.control_height + 8 ? detail::kLevelIconSize : t.metrics.control_height + 8);
    return t.metrics.titlebar_height + t.metrics.window_inset + body_h +
           t.metrics.window_inset + t.metrics.control_height + t.metrics.window_inset;
}

void MessageBox::Notify(DuiLib::TNotifyUI& msg) {
    if (msg.sType != _T("click")) {
        return;
    }
    const CDuiString& name = msg.pSender->GetName();
    if (name == _T("mb_close")) {
        Finish(cancel_result_);
        return;
    }
    if (name.GetLength() > 7 && name.Left(7) == _T("mb_btn_")) {
        Finish(static_cast<MessageResult>(_ttoi(name.Mid(7).GetData())));
    }
}

LRESULT MessageBox::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        if (wParam == VK_ESCAPE) {
            Finish(cancel_result_);
            return 0;
        }
        if (wParam == VK_RETURN && default_result_ != MessageResult::None) {
            Finish(default_result_);
            return 0;
        }
    }
    return duilib::FramelessWindow::HandleMessage(uMsg, wParam, lParam);
}

void MessageBox::OnFinalMessage(HWND) {
    if (modeless_) {
        delete this;
    }
}

void MessageBox::Finish(MessageResult result) {
    Close(static_cast<UINT>(result));
}

MessageResult MessageBox::Show(HWND owner, const MessageBoxSpec& spec) {
    auto* box = new MessageBox(spec);
    // 独立窗体：不设 owner，WS_EX_APPWINDOW 保有任务栏位；归属窗体相反。
    const DWORD ex_style = WS_EX_WINDOWEDGE | (spec.owned ? 0 : WS_EX_APPWINDOW);
    const DWORD style = (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX)) | WS_VISIBLE;
    HWND hwnd = box->Create(spec.owned ? owner : nullptr, spec.title, style, ex_style);
    if (hwnd == nullptr) {
        delete box;
        return MessageResult::None;
    }
    box->ResizeClient(scale(&box->paint_manager(), spec.width),
                      scale(&box->paint_manager(), box->EstimateContentHeight()));
    box->CenterWindow();
    if (!spec.modal) {
        box->modeless_ = true;
        ::ShowWindow(hwnd, SW_SHOW);
        ::UpdateWindow(hwnd);
        return MessageResult::None;
    }
    // ShowModal 经 GetWindowOwner 恢复使能；独立窗体无 owner，焦点手动归还。
    const UINT ret = box->ShowModal();
    ::DestroyWindow(hwnd);
    if (owner != nullptr && ::IsWindow(owner)) {
        ::SetFocus(owner);
    }
    delete box;
    return static_cast<MessageResult>(ret);
}

}  // namespace uikit
