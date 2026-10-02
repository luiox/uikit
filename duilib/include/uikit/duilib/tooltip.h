#pragma once

// 工具提示（L3，uikit 顶层命名空间）：全新自绘主题气泡，到时自消。

#include <UIlib.h>

#include "uikit/duilib/frameless.h"

namespace uikit {

// 自绘气泡，不走 fork 原生 SetToolTip 机制——那条链路由 manager 挂系统
// TOOLTIPS_CLASS 窗口承接（UIManagerWin32 的 TTM_ADDTOOL），配色/形状不可
// 主题化。形态照抄 toast.cpp 配方：WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW +
// WS_POPUP 的 FramelessWindow 顶层窗体，不抢焦点、不进任务栏/Alt-Tab，
// duration（默认 3000ms）到时自关并自删（Show 不返回句柄）。
// 外观：panel 底 + 1px 描边 + 正文色文本，圆角随 radius_control 令牌；
// 最大宽 240 逻辑像素，超宽折行（行数估算与 toast/messagebox 同口径）。
// 定位：target 下方 8 逻辑像素（左缘对齐），越出屏幕工作区翻转到上方。
// 悬停自动触发不内建（需要子类化任意控件/改公共头）：宿主可在控件的
// mousehover 事件（TrackMouseEvent）里自行调用 Show，配计时防抖即可。
class Tooltip : private duilib::FramelessWindow {
public:
    // target 须已挂窗排版（GetPos 换算屏幕坐标依赖宿主窗口）；owner 为空时
    // 落到 target 宿主窗口。text 为空或 target 未排版则不出泡。
    static void Show(HWND owner, DuiLib::CControlUI* target, const DuiLib::CDuiString& text,
                     int duration_ms = 3000);

private:
    Tooltip(const DuiLib::CDuiString& text, int duration_ms);
    LPCTSTR GetWindowClassName() const override { return _T("UIKitTooltip"); }
    DuiLib::CControlUI* BuildRootUi() override;
    void Notify(DuiLib::TNotifyUI& msg) override { (void)msg; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    void OnFinalMessage(HWND hWnd) override;

    void CloseSelf();

    DuiLib::CDuiString text_;
    int duration_ms_;
    bool closing_ = false;
};

}  // namespace uikit
