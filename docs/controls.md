# 控件

全部控件在 `uikit/duilib/controls.h`，绘制时直读 `ActiveTheme()`，
切主题无需重建控件（构造期上色项靠 `ApplyThemeToTree` 重涂）。

## 按钮族

```cpp
auto* save = new ButtonUI();
save->SetText(_T("保存"));
save->StylePrimary();          // accent 底 + on_accent 文字，圆角 radius_control
auto* cancel = MakeTextButton(_T("cancel"), _T("取消"), 72);   // secondary 预设

auto* btn = new ButtonUI();
btn->SetStateColors(n, h, p);  // 显式状态色（mlaunch 迁移兼容），此后不随主题预设
btn->SetActive(true);          // 粘滞按下态（如搜索模式放大镜）
```

`IconButtonUI`：等大正方形（metrics.icon_button_size），常态透明悬停显灰底；
SVG 图经 `SetSvgImage(image_attr)` 挂 normalimage/hotimage/pushedimage
（图标资产来自 micon，语义名映射见 `design/icons.json`）。

## 勾选 / 单选

```cpp
auto* check = new CheckBoxUI();
check->SetText(_T("开机自启"));
check->SetChecked(true);
// IsChecked() 读态；点击自切换并发 click 通知

auto* light = new RadioButtonUI();
auto* dark  = new RadioButtonUI();
DuiLib::CControlUI* group[] = {light, dark};
light->SetGroup(group, 2);  dark->SetGroup(group, 2);   // 组内互斥
```

自绘实现（fork 的 CCheckBoxUI 无资源不可见）：勾选盒 metrics.checkbox_box
按 DPI 缩放，accent 底 + 圆角 + on_accent 对勾；未勾选 border_strong，
悬停 border_focus；禁用整体退为 control_disabled。

## 输入框

```cpp
auto* edit = new EditUI();
edit->SetTipValue(_T("输入内容，回车确认…"));   // 占位提示（text_secondary 色）
edit->SetTipValueColor(DuiLib::CDuiColor(0xFF8A8A8A));  // 可选；默认取主题
auto* pwd = new EditUI();
pwd->SetPassword(true);
```

surface 底圆角，高 metrics.control_height、内边距 metrics.control_hpad；
边框状态机：禁用 control_disabled → 聚焦 accent → 悬停 border_focus →
常态 border。原生 EDIT 子窗跟随主题底色（构造期 `OnThemeChanged` 上色）。

## 进度条

```cpp
auto* bar = new ProgressBarUI();   // 默认 0..100、高 metrics 半高胶囊
bar->SetValue(35);
// SetMinValue/SetMaxValue 改量程；越界值被拒绝（fork 语义，调用方先钳位）
```

轨道 = surface 胶囊（宽随行高），填充 = accent 圆角条；两段自绘于
`PaintForeColor`，无图片属性依赖，绘制时直读主题。

## 消息框

`MessageBoxUI` 在 `uikit/duilib/messagebox.h`（FramelessWindow 子类，非
controls.h）。两维**正交**：modal/modeless × owned/独立 HWND——独立窗体
不挂在宿主 HWND 树下，可越出嵌入 surfaces 的边界做对话框。

```cpp
MessageBoxSpec spec;
spec.title  = _T("删除确认");
spec.text   = _T("配置文件无法解析，是否重建？未保存的修改将丢失。");
spec.level  = MessageLevel::Warning;      // Info/Success/Warning/Error/Question
spec.buttons = MessageButtons::YesNo;     // Ok/OkCancel/YesNo/YesNoCancel/RetryCancel/AbortRetryIgnore
spec.modal  = true;    // true=阻塞返回结果；false=立即返回 None
spec.owned  = true;    // false=独立 HWND（WS_EX_APPWINDOW，可盖在宿主之外）
MessageResult r = MessageBoxUI::Show(hwnd, spec);   // 数值同 IDOK/IDYES…
```

级别图标为自绘圆盘 + 字形（无贴图；success/warning/danger 语义色来自
L1 令牌）。ESC 返回 cancel 语义键、Enter 返回默认键；模态关闭后焦点
归还 owner。

## 文本 / 面板

```cpp
auto* title = new LabelUI();                       // 正文色
auto* hint  = new LabelUI();
hint->SetRole(LabelUI::Role::Secondary);           // 弱化色（派生）
auto* root = new PanelUI();                        // bkcolor 恒等于主题 panel
```

## 列表 / 滚动条

```cpp
auto* groups = new GroupListUI();   // panel 底、选中条 panel_selected
auto* items  = new ItemListUI();    // surface 底、选中条 surface_selected
ApplyFlatScrollbar(groups, ActiveTheme().color.panel);   // 极简轨道，无箭头
// thumb 九宫格图（可选第三参）：用 micon 的 MakeScrollbarThumbAttr 同款产物
```

`GroupRowUI`：选中时行末补画 1px row_selected_border 描边。

## 输入 / 顶栏

`SearchBoxUI`：surface 底无边框，高 metrics.search_height，内边距
metrics.control_hpad。`TitleBarUI`：panel 底、高 metrics.titlebar_height、
childvalign center——配合 `FramelessWindow` 使用，见 [frameless.md](frameless.md)。
