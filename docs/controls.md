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

## 按钮 / 勾选 / 列表 / 顶栏（包装层）

见 `uikit/duilib/controls.h`：`ButtonUI`（StylePrimary/Secondary）、
`IconButtonUI`、`MakeTextButton`、`CheckBoxUI` / `RadioButtonUI`（自绘
accent 勾选盒/外环圆点）、`LabelUI`（正文/弱化）、`PanelUI`、
`SearchBoxUI`、`TitleBarUI`、`GroupListUI` / `ItemListUI` / `GroupRowUI`、
`ApplyFlatScrollbar`。无边框窗口配方见 [frameless.md](frameless.md)。

## 未做（缺件路线图）

| 控件 | 状态 | 说明 |
| ---- | ---- | ---- |
| TabControl | 未做 | 页签 + CTabLayoutUI 组合；页签头自绘（选中下划线 accent） |
| Menu / ContextMenu | 未做 | fork 有 CMenuUI/CMenuWnd 全套，缺主题化包装（弹层配色 + 分隔线） |
| Tooltip | 未做 | fork 原生 SetToolTip 可用，缺主题化气泡（manager 级配色） |
| DatePicker | 未做 | 日历弹层体量大，押后；先以 SpinBox/ComboBox 覆盖常用场景 |
