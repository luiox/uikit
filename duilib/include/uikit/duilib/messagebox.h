#pragma once

// 消息框预制件（L3 组合层）：frameless 顶层窗口 + 主题化级别图标/按钮组。
// 对话框的两个正交维度由 spec 显式给出：
// · modal（模态/非模态）：true = EnableWindow(owner) + 本地消息循环，Show()
//   返回点击结果；false = ShowWindow 后立即返回 None，窗口自管理生命周期
//   （关闭时自删，调用方不得再 delete）。
// · owned（归属/独立）：true = 以 owner 为 Win32 owner——无任务栏项、恒压在
//   owner 之上、随 owner 消亡；false = 独立顶层 HWND——不设 owner，可拖出
//   owner 范围独立存活、有自己的任务栏位（duilib 面板嵌在宿主里的场景，
//   "对话框逃出宿主表面"即此形态）。两维正交，四种组合均合法。
//
// 命名：面向消费方的新式控件/预制件直接挂 uikit 顶层命名空间（无 UI 后缀，
// uikit::MessageBox）；uikit::duilib 保留给包装层细节与既有控件。

#include <UIlib.h>

#include <vector>

#include "uikit/duilib/frameless.h"
#include "uikit/theme/theme.h"

namespace uikit {

enum class MessageLevel { Info, Success, Warning, Error, Question };
enum class MessageButtons { Ok, OkCancel, YesNo, YesNoCancel, RetryCancel, AbortRetryIgnore };

// 数值与 Win32 IDOK/IDCANCEL/... 同值，宿主可直接与 ::MessageBox 返回值混用。
enum class MessageResult {
    None = 0,
    Ok = 1,
    Cancel = 2,
    Abort = 3,
    Retry = 4,
    Ignore = 5,
    Yes = 6,
    No = 7,
};

struct MessageBoxSpec {
    DuiLib::CDuiString title;
    DuiLib::CDuiString text;
    MessageLevel level = MessageLevel::Info;
    MessageButtons buttons = MessageButtons::Ok;
    bool modal = true;
    bool owned = true;
    int width = 360;  // 96 基准逻辑像素；高度按文本行数估算
};

class MessageBox : public duilib::FramelessWindow {
public:
    // modal 阻塞至用户选择并返回结果；modeless 立即返回 None。
    // owner 可为 NULL（此时 owned 维度无意义，窗口天然独立）。
    static MessageResult Show(HWND owner, const MessageBoxSpec& spec);

    LPCTSTR GetWindowClassName() const override { return _T("UIKitMessageBox"); }

protected:
    DuiLib::CControlUI* BuildRootUi() override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    // 键盘：Enter → 默认项，Esc → 取消项（无取消项的按钮组仅关闭）。
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    void OnFinalMessage(HWND hWnd) override;

private:
    struct ButtonDef {
        MessageResult result;
        LPCTSTR text;
        bool primary;
    };

    explicit MessageBox(const MessageBoxSpec& spec);
    void BuildButtonDefs(std::vector<ButtonDef>& out);
    void BuildButtonRow(DuiLib::CContainerUI* row, const std::vector<ButtonDef>& defs);
    int EstimateContentHeight();
    void Finish(MessageResult result);

    MessageBoxSpec spec_;
    std::vector<ButtonDef> buttons_;
    MessageResult default_result_ = MessageResult::None;
    MessageResult cancel_result_ = MessageResult::None;
    bool modeless_ = false;
};

}  // namespace uikit
