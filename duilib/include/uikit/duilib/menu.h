#pragma once

// 弹出菜单（L3，uikit 顶层命名空间）：FramelessWindow 顶层弹层 + 阻塞 Show。
//
// 路线说明（为何不走 fork 的 CMenuUI/CMenuWnd）：duilib 包里确有 UIMenu.h/
// UIMenuWndWin32.h，但那条路是 XML 驱动——CMenuWndWin32::CreateMenu(pOwner,
// xml, point) 要求宿主窗口内嵌 CMenuElementUI 并提供菜单 XML 资源，条目皮肤
// （底图/悬停图/勾选图）依赖图片资产。与本项目「全自绘无图片、绘制期直读
// ActiveTheme()、Show 只收裸 HWND + 条目数组」的约束都不兼容，故照 toast/
// messagebox 的 FramelessWindow 配方自研弹层，主题化彻底（描边/圆角/内边距/
// 悬停色全走令牌）。条目行无图标位，后续可接 micon SVG 语义图标。

#include <UIlib.h>

#include <vector>

#include "uikit/duilib/frameless.h"
#include "uikit/theme/theme.h"

namespace uikit {

// 单条菜单项。separator_after = 该项下画 1px 分隔线（divider 令牌色）。
// id 需为非负值：-1 由 Menu::Show 保留为"未选择"信号（Esc/点击外部/失焦）。
struct MenuItem {
    DuiLib::CDuiString text;
    int id = 0;
    bool enabled = true;
    bool separator_after = false;
};

// 阻塞式弹出菜单：Show 照 MessageBox::Show 的模态口径（EnableWindow(owner,
// false) + 本地消息循环，即 fork CWindowWin32::ShowModal 的展开版）阻塞至
// 选择/关闭，返回所选 id；未选（Esc / 点击外部 / 失焦）返回 -1。
// 弹层要抢键盘与焦点（Esc、↑/↓、Enter 生效），故不走 toast 的
// WS_EX_NOACTIVATE；TOOLWINDOW 不进任务栏，TOPMOST 压过宿主。
// 点击外部关闭 = SetCapture 捕获「按下点在窗体外」+ WM_KILLFOCUS 双保险：
// owner 被 EnableWindow(false) 禁用后点击它不会移动焦点，失焦路径不触发，
// 捕获路径兜底（fork 菜单同款 SetForegroundWindow + KILLFOCUS 配方的补全）。
class Menu : private duilib::FramelessWindow {
public:
    // screen_pt 为屏幕坐标（通常是右键点或锚点），阻塞返回所选 id。
    static int Show(HWND owner, const POINT& screen_pt, const std::vector<MenuItem>& items);

private:
    explicit Menu(const std::vector<MenuItem>& items);

    LPCTSTR GetWindowClassName() const override { return _T("UIKitMenu"); }
    DuiLib::CControlUI* BuildRootUi() override;
    // 条目行经 Activate 自持回调（spinbox 的 StepButton 同款），不经 Notify。
    void Notify(DuiLib::TNotifyUI& msg) override { (void)msg; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    // 内部条目行（menu.cpp 内定义），键盘高亮/选中要直接操作行。
    class Row;
    friend class Row;

    void Pick(int id);              // 选中条目 → Close(id)
    void Cancel();                  // 未选关闭 → Close(-1)
    void MoveHighlight(int delta);  // 键盘高亮移动（跳过禁用项）
    int MeasureMaxTextWidth();      // 实测最长条目文本宽（设备像素）

    const std::vector<MenuItem> items_;
    std::vector<Row*> rows_;  // 与 items_ 一一对应（分隔线是独立 1px 控件）
    int kb_index_ = -1;       // 键盘高亮行索引，-1 = 无
    bool closed_ = false;     // Close 幂等护栏（同帧 KILLFOCUS/外点多路径触发）
};

}  // namespace uikit
