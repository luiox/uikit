#pragma once

// 悬浮通知（L3，uikit 顶层命名空间）：无焦点浮层，到时自消。

#include <UIlib.h>

#include "uikit/duilib/frameless.h"
#include "uikit/duilib/messagebox.h"  // MessageLevel
#include "uikit/theme/theme.h"

namespace uikit {

struct ToastSpec {
    DuiLib::CDuiString text;
    MessageLevel level = MessageLevel::Info;
    int duration_ms = 2200;  // 到时自动关闭
    int width = 320;         // 96 基准逻辑像素；高度按文本行数估算
};

// 无焦点、不抢活动的顶层通知：WS_EX_NOACTIVATE + TOOLWINDOW（无任务栏项），
// 显示 duration_ms 后自关闭并自删（Show 不返回句柄，调用方无从持有）。
// 位置：owner 居中上方（owner 为 NULL 时落主屏工作区顶中）；同屏多例自动
// 纵向错开。级别图标与消息框共用同一自绘实现。
class Toast : private duilib::FramelessWindow {
public:
    static void Show(HWND owner, const ToastSpec& spec);

private:
    explicit Toast(const ToastSpec& spec);
    LPCTSTR GetWindowClassName() const override { return _T("UIKitToast"); }
    DuiLib::CControlUI* BuildRootUi() override;
    void Notify(DuiLib::TNotifyUI& msg) override { (void)msg; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    void OnFinalMessage(HWND hWnd) override;

    void CloseSelf();

    ToastSpec spec_;
    bool closing_ = false;
};

}  // namespace uikit
