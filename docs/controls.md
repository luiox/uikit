# 控件

两套命名层次：

- **`uikit` 顶层（新式，推荐）**：面向消费方的控件与预制件，无 UI 后缀
  （`uikit::Edit`、`uikit::MessageBox`），头文件按控件分立。
- **`uikit::duilib`（包装层）**：既有控件与基础设施（`ButtonUI`、
  `ThemeableControl`、`FramelessWindow`…），见 `controls.h`/`frameless.h`。

全部绘制时直读 `ActiveTheme()`，切主题 = `SetActiveTheme()` +
`ApplyThemeToTree()`，无需重建控件。

## 默认外观：直角密度优先

统一默认外观走"前端式简约 + native 密度"：圆角令牌 `radius_control` /
`radius_checkbox` 默认 **0（直角）**，深浅主题一致。控件不再各自发明圆角，
需要圆角外观时改主题令牌即可全局生效（控件内的圆角一律取自令牌）。

## 输入框

```cpp
#include <uikit/duilib/edit.h>
auto* edit = new uikit::Edit();
edit->SetTipValue(_T("输入内容，回车确认…"));   // 占位提示（text_secondary 色）
auto* pwd = new uikit::Edit();
pwd->SetPasswordMode(true);
```

surface 直角底 + 状态边框：禁用 control_disabled → 聚焦 accent →
悬停 border_focus → 常态 border。原生 EDIT 子窗跟随主题底色
（构造期 `OnThemeChanged` 上色；native 刷子不动的约束见源码注释）。

## 进度条

```cpp
#include <uikit/duilib/progress.h>
auto* bar = new uikit::ProgressBar();   // 默认 0..100、高 6
bar->SetValue(35);
// SetMinValue/SetMaxValue 改量程；越界值被拒绝（fork 语义，调用方先钳位）
```

轨道 = surface + accent 填充，圆角随 `radius_control` 令牌；
两段自绘于 `PaintForeColor`，无图片属性依赖。

## 开关

```cpp
#include <uikit/duilib/switch.h>
auto* sw = new uikit::Switch();
sw->SetChecked(true);   // IsChecked() 读态；点击自切换并发 click
```

开 = accent 轨道 + on_accent 滑块，关 = border_strong 轨道；悬停加深、
禁用退色。轨道圆角随令牌（默认直角滑块样式）。

## 滑杆

```cpp
#include <uikit/duilib/slider.h>
auto* slider = new uikit::Slider();
slider->SetValue(40);          // min/max/value 沿 CProgressUI
```

4px 直角轨道 + accent 已选段 + accent 圆形滑块（on_accent 细环）。
拖拽/点击定位由 fork 的 CSliderUI 承接（SetThumbSize 与视觉一致保证命中）。

## 下拉选择

```cpp
#include <uikit/duilib/combobox.h>
auto* combo = new uikit::ComboBox();
combo->AddOption(_T("自动（推荐）"));
combo->AddOption(_T("1920 × 1080"));
combo->SelectItem(0, false, false);   // 第三个参 false = 不触发 itemselect
```

fork 的 CComboUI 承接弹出/选择/可编辑（SetDropType）语义；本层做主题化：
surface 直角底 + 状态边框、弹层条目 surface 实底 + hover/selected 色、
自绘下拉箭头（fork 的 db 系列走图片资产，无资源不可见——同按钮族缺口）。

## 步进器

```cpp
#include <uikit/duilib/spinbox.h>
auto* spin = new uikit::SpinBox();
spin->SetValueRange(1, 72);
spin->SetValue(12);
spin->SetStep(2);
```

[−] Edit [+] 复合布局；−/+ 点击自持（不依赖宿主 Notify），Edit 手输经
fork 的控件级 OnNotify 挂点在 return/killfocus 时解析钳位。每次值变化向
窗口发 `"valuechange"` 通知（wParam = 新值）。

## 悬浮通知

```cpp
#include <uikit/duilib/toast.h>
ToastSpec spec;
spec.text  = _T("设置已保存。");
spec.level = MessageLevel::Success;   // 图标与消息框同套自绘
Toast::Show(hwnd, spec);              // 即发即忘，duration_ms(默认2200) 后自消
```

无焦点浮层（WS_EX_NOACTIVATE + TOOLWINDOW，不抢活动、不进任务栏），
owner 居中上方，同屏多例自动纵向错开；窗口自管理生命周期。

## 消息框

`uikit::MessageBox` 在 `uikit/duilib/messagebox.h`（FramelessWindow 子类）。
两维**正交**：modal/modeless × owned/独立 HWND——独立窗体不挂在宿主
HWND 树下，可越出嵌入 surfaces 的边界做对话框。

```cpp
MessageBoxSpec spec;
spec.title  = _T("删除确认");
spec.text   = _T("配置文件无法解析，是否重建？未保存的修改将丢失。");
spec.level  = MessageLevel::Warning;      // Info/Success/Warning/Error/Question
spec.buttons = MessageButtons::YesNo;     // Ok/OkCancel/YesNo/YesNoCancel/RetryCancel/AbortRetryIgnore
spec.modal  = true;    // true=阻塞返回结果；false=立即返回 None
spec.owned  = false;   // false=独立 HWND（WS_EX_APPWINDOW，可盖在宿主之外）
MessageResult r = MessageBox::Show(hwnd, spec);   // 数值同 IDOK/IDYES…
```

级别图标为自绘圆盘 + 字形（success/warning/danger 语义色来自 L1 令牌，
与 Toast 共用同一实现）。ESC 返回 cancel 语义键、Enter 返回默认键；
模态关闭后焦点归还 owner。

## 页签

```cpp
#include <uikit/duilib/tabcontrol.h>
auto* tabs = new uikit::TabControl();
tabs->AddPage(page1, _T("常规"));   // page 为任意控件（PanelUI 等）
tabs->AddPage(page2, _T("高级"));
tabs->SetFixedHeight(160);
```

页签头自绘（选中 = 正文色 + accent 下划线，未选中弱化、悬停正文），
内容区走包内 CTabLayoutUI 联动；页签宽自适应文本（下限 64）。选页内部
自持（宿主零接线），并向窗口发 `"tabchange"` 通知（wParam = 页索引）；
fork 的 CTabLayoutUI 自带 `"tabselect"` 通知与页内聚焦，可二选一监听。

## 菜单

```cpp
#include <uikit/duilib/menu.h>
std::vector<MenuItem> items = {
    {_T("打开"), 1, true, false},
    {_T("另存为…"), 2, true, true},   // separator_after
    {_T("删除"), 3, true, false},
};
const int picked = Menu::Show(hwnd, screen_pt, items);  // 阻塞，未选返回 -1
```

自研弹层（fork 的 CMenuUI 是 XML+图片驱动，与无图片约束不兼容——见
准入清单）。阻塞式（模态口径：禁用 owner + 本地循环 + 焦点归还），
键盘 ↑/↓/Enter/Esc 可用；surface 底 1px 描边、悬停 surface_hover、
分隔线 divider、宽度自适应最长条目（下限 160）。点击外部关闭 =
SetCapture 外点捕获 + WM_KILLFOCUS 双保险。

## 气泡提示

```cpp
#include <uikit/duilib/tooltip.h>
Tooltip::Show(hwnd, target_control, _T("提示文本"));   // 即发即忘
```

自绘气泡（fork 原生 SetToolTip 走系统 TOOLTIPS 窗口，不可主题化）：
panel 底 + 1px 描边、最大宽 240 自动折行、NOACTIVATE 不抢焦点，出现在
目标控件下方 8 逻辑像素（越出工作区上翻），默认 3000ms 自消。
悬停自动触发不内建：宿主在控件 mousehover 事件里调 Show 即可
（后续可加 Cancel/自动挂钩，属公共头扩展）。

## 日期选择

```cpp
#include <uikit/duilib/datepicker.h>
auto* date = new uikit::DatePicker();      // 默认今天
date->SetDate(2026, 10, 1);
int y = date->GetYear();                   // GetMonth/GetDay/GetDateYmd
date->ShowCalendar();                      // 程序主动展开日历（F4 惯例）
// 确认选日发 "datechange"（wParam = yyyymmdd）；翻页不通知
```

只读显示框 + 日历按钮复合；日历弹层（NOACTIVATE+TOOLWINDOW，不抢宿主
焦点）：‹ › 翻月、星期表头、6×7 网格——今天 accent 描边、选中 accent 底
白字、相邻月弱化。只读实现：拦截点击/聚焦事件使原生 EDIT 子窗不创建
（包内 CEditUI 无 readonly 能力）。Esc 不支持（NOACTIVATE 收不到键盘，
点外部/再点按钮收起）。

## 按钮 / 勾选 / 列表 / 顶栏（包装层）

见 `uikit/duilib/controls.h`：`ButtonUI`（StylePrimary/Secondary）、
`IconButtonUI`、`MakeTextButton`、`CheckBoxUI` / `RadioButtonUI`（自绘
accent 勾选盒/外环圆点）、`LabelUI`（正文/弱化）、`PanelUI`、
`SearchBoxUI`、`TitleBarUI`、`GroupListUI` / `ItemListUI` / `GroupRowUI`、
`ApplyFlatScrollbar`。无边框窗口配方见 [frameless.md](frameless.md)。

## 剩余事项（非缺件，属增强）

| 事项 | 说明 |
| ---- | ---- |
| Menu 子菜单 | 弹层菜单目前单层；子菜单需要级联展开与悬停延迟 |
| Tooltip 自动挂钩 | 自动 mousehover 触发需公共头扩展（Cancel API + 挂钩约定） |
| 图标位 | Menu/Toast/MessageBox 条目留了加图标位的余地，待接 micon SVG |
| 高 DPI 网格 | 属性几何用逻辑值、窗口尺寸 scale 的既有约定下，scale≠1 时日历列有轻微错位，待统一 |
