#pragma once

// 页签（L3，uikit 顶层命名空间）：自绘页签头 + 内容区联动。

#include <UIlib.h>

#include <vector>

#include "uikit/duilib/controls.h"

namespace uikit {

// 复合布局：页签头（panel 底，高 control_height）+ 内容区（CTabLayoutUI，
// 吃剩余空间、不设固定高）。内容区用包内 CTabLayoutUI 承接"单页可见"语义，
// 页签头完全自绘：选中 = 正文色 + accent 下划线（3 逻辑像素、整页签宽），
// 未选中 = text_secondary，悬停 = 正文色；页签间无分隔线，圆角一律随
// radius_control 令牌（当前主题为 0 = 直角）。
// 页签钮是内部子类（拦截 Activate，同 SpinBox::StepButton 模式），点击自持
// 切页、宿主无须接线；选页变化对窗口发 "tabchange" 通知（wParam = 页索引）。
// 内容区另会收到 fork CTabLayoutUI 原生的 "tabselect" 通知，宿主二选一监听。
class TabControl : public DuiLib::CVerticalLayoutUI, public duilib::ThemeableControl {
public:
    TabControl();
    LPCTSTR GetClass() const override { return _T("UIKitTabControl"); }
    void OnThemeChanged() override;

    // 追加一页（page 生命周期随控件树，宿主先排好页内布局再挂入）。
    // 首个加入的页自动成为当前页（CTabLayoutUI 语义，这里同步页签头）。
    void AddPage(DuiLib::CControlUI* page, LPCTSTR title);
    // 切页并联动内容区与页签头；越界/同页为 no-op，不同页才发 "tabchange"。
    void SetCurSel(int index);
    int GetCurSel() const { return cur_sel_; }

private:
    // 页签钮：透明底 + 状态文本色 + 选中下划线，点击回调宿主切页。
    class TabButton;

    void UpdateButtons();

    DuiLib::CHorizontalLayoutUI* header_ = nullptr;
    DuiLib::CTabLayoutUI* pages_ = nullptr;
    std::vector<TabButton*> tabs_;
    int cur_sel_ = -1;
};

}  // namespace uikit
