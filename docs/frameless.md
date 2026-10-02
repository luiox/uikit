# 无边框窗口

基础无边框（caption 拖动、NCHITTEST 缩放）由 duilib fork 原生承接；
`uikit/duilib/frameless.h` 只是把"创建配方"打包成可继承的模板：

1. `StripCaption`：剥 WS_CAPTION（mlaunch 同款）；
2. `OnCreate`：Init **基类自带的 m_pm** → 装主题字体 → `BuildRootUi()` →
   AttachDialog → SetCaptionRect(titlebar 高) / SetSizeBox(sizebox)；
3. `OnNcCalcSize/OnNcPaint`：吞掉原生 NC 区（否则 DWM 在 WS_THICKFRAME 下
   会画默认标题栏）；
4. `OnNcHitTest`：sizebox 边缘返回缩放命中；caption 矩形内命中链无交互控件
   （按钮/输入/列表等）返回 HTCAPTION 供拖拽，有则返回 HTCLIENT 放行点击。

## 用法

```cpp
class SettingsWindow : public uikit::duilib::FramelessWindow {
    LPCTSTR GetWindowClassName() const override { return _T("SettingsWindow"); }
    DuiLib::CControlUI* BuildRootUi() override {
        auto* root = new PanelUI();
        root->Add(new TitleBarUI());
        // ... 主体控件
        return root;
    }
    void Notify(DuiLib::TNotifyUI& msg) override { /* click 通知 */ }
};

SettingsWindow w;
w.Run(_T("设置"), 480, 360);   // CoInitialize 之后调用
```

## 完整预制件：FrameWindow（带窗口钮）

标题栏 + 最小化/最大化/关闭钮的整套装配，"主题配套无边框窗口"的
一站式用法——子类只填内容区，不再手拼标题栏：

```cpp
#include <uikit/duilib/frame_window.h>

class SettingsWindow : public uikit::FrameWindow {
    LPCTSTR GetWindowClassName() const override { return _T("SettingsWindow"); }
    void InitContent(DuiLib::CContainerUI* content) override {
        // content = 标题栏下的空 PanelUI，吃剩余空间
        content->Add(/* 主体控件 */);
    }
};

SettingsWindow w;
w.SetTitleText(_T("设置"));
w.ShowMaximizeButton(false);   // 可选：显隐最小化/最大化钮
w.Run(_T("设置"), 480, 360);
```

- 窗口钮自绘：悬停底 icon_hot、**close 悬停 danger + on_accent 字形**
  （native 语义）；字形为几何线框（min 一线 / max 一框 / restore 双框 /
  close 交叉线），随主题文字色，直角满条高。
- 最大化/还原切换自动换字形；最大化态 NC 框厚已修正（内容不越出工作区），
  边缘缩放命中由基类的 IsZoomed 守卫关闭。
- `fw_min/fw_max/fw_close` 点击内部自持；模态/非模态两种宿主方式均可用
  （模态走 `Run`/`ShowModal`，非模态堆分配 + `OnFinalMessage` 自删）。

## 边界（谁修什么）

消息层修补（窗口阴影、Win11 圆角与 Snap Layout、最大化遮挡任务栏、
Per-Monitor DPI 抖动、最小化动画）归 DuiLib fork 的 PR 流程，
uikit 不私改消息语义；uikit 这层只做"组合与默认值"。
