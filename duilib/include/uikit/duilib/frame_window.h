#pragma once

// 主题配套的完整无边框窗口预制件（L3 组合层）：标题栏（标题 + 最小化/
// 最大化/关闭自绘窗口钮）+ 内容区槽位。无边框基础（剥 caption、NCHITTEST
// 缩放、caption 拖拽/双击最大化、交互控件放行）由 FramelessWindow 承接，
// 本层只装配标题栏与窗口态按钮——mlaunch/demo 手拼标题栏的重复配置到此收口。

#include <UIlib.h>

#include "uikit/duilib/frameless.h"

namespace uikit {

class FrameWindow : public duilib::FramelessWindow {
public:
    // 均须在 Run/Create 之前设置（BuildRootUi 装配期消费）。
    void SetTitleText(LPCTSTR text);
    void ShowMinimizeButton(bool show);
    void ShowMaximizeButton(bool show);

protected:
    // 子类填充内容区；content 已挂到标题栏之下（PanelUI，吃剩余空间）。
    virtual void InitContent(DuiLib::CContainerUI* content) = 0;

    DuiLib::CControlUI* BuildRootUi() override;
    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    // 还原态沿用基类的 sizebox 命中；最大化态补 NC 修正（见 cpp）。
    LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

private:
    class CaptionButton;  // 自绘窗口钮：min/max/restore 字形 + 悬停底（close=danger）

    void ToggleMaximize();
    void UpdateMaxGlyph();

    DuiLib::CDuiString title_;
    CaptionButton* min_button_ = nullptr;
    CaptionButton* max_button_ = nullptr;
    bool show_min_ = true;
    bool show_max_ = true;
    bool zoomed_ = false;
};

}  // namespace uikit
