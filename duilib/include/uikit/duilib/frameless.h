#pragma once

// 无边框窗口预制件（L3 组合层）。基础无边框（caption 拖动、NCHITTEST 缩放）
// 由 duilib fork 原生承接——本层只是把"创建配方"打包成模板：
// 剥 WS_CAPTION → Init PaintManager → 装字体 → BuildRootUi → AttachDialog →
// SetCaptionRect/SetSizeBox。消息语义修补（阴影/Win11 圆角/DPI 抖动）归 fork
// PR 流程，不在这里私改（PLAN.md「无边框边界」）。

#include <UIlib.h>

#include "uikit/duilib/applier.h"
#include "uikit/theme/theme.h"

namespace uikit::duilib {

// 从窗口样式里剥掉原生标题栏（mlaunch 同款配方，fork 的 NCHITTEST 承接缩放）。
void StripCaption(HWND hwnd);

// 无边框窗口基类：子类实现 BuildRootUi()（用 uikit 控件拼界面）与
// GetWindowClassName()，Notify() 处理业务通知。
// 注意：PaintManager 用基类 CWindowWnd 自带的 m_pm（渲染链路只认它），
// 自建 manager 成员会出现"窗口活着但永远白屏"的错位。
class FramelessWindow : public DuiLib::CWindowWin32, public DuiLib::INotifyUI {
public:
    // 创建并进入消息循环。title/caption_h_px/sizebox 均为 96 基准逻辑像素。
    int Run(LPCTSTR title, int width, int height, int caption_h_px = 30, int sizebox_px = 6);

    DuiLib::CPaintManagerUI& paint_manager() { return m_pm; }
    DuiLib::CControlUI* FindControl(LPCTSTR name) { return m_pm.FindControl(name); }

protected:
    LPCTSTR GetWindowClassName() const override = 0;
    virtual DuiLib::CControlUI* BuildRootUi() = 0;
    // 字体尺寸（默认取主题 default 字号），有 DPI 需求时子类可覆写。
    virtual int DefaultFontPx() const;
    // 由基类 HandleMessage 分发（勿在子类手拦 WM_CREATE，会双次初始化）。
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
    // 吞掉原生 NC 区（否则 DWM 会在 WS_THICKFRAME 下画默认标题栏）。
    LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
    LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
    // WindowImplBase 同款无边框配方：sizebox 边缘缩放 + caption 区拖拽，
    // 交互控件（按钮/输入/列表）所在处返回 HTCLIENT 放行点击。
    LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

    int caption_h_px_ = 30;
    int sizebox_px_ = 6;
};

}  // namespace uikit::duilib
