# 技术栈决策复核（基于 VB6 反编译）

> 日期：2026-02-20  
> 目标：结合旧版 Poner（VB6）真实实现，判断 nassistant 是否应继续 Tauri，还是切换技术栈。

---

## 1. 先给结论

结论分两层：

1. **短期（1~2 个版本）**：可以继续 Tauri，但必须做窗口架构止损（单窗口化 + 去复杂自绘窗口效果）。
2. **中期（追求你定义的高性能/高密度上限）**：建议迁移到原生 UI 路线；优先 **Rust + Slint**，若你接受更高维护成本再考虑 **Win32/duilib**。

一句话：
**你现在的目标是“工具级原生质感 + 高密度 + 低占用”，长期看 WebView 不是最优解；但立刻全量重写成本太高，建议“先止损，再迁移”。**

---

## 2. 旧版 VB6 的关键技术事实（来自 dec）

以下是你旧版启动器“体感快、原生感强”的关键来源，不是猜测：

- **大量 GDI 直接绘制（BitBlt/AlphaBlend）**：在 [dec/UI.ctl](dec/UI.ctl) 与 [dec/Tabs.ctl](dec/Tabs.ctl) 有大量 `BitBlt` 调用，说明核心 UI 是自绘位图拼接，不依赖浏览器布局引擎。
- **分层窗口/透明控制**：在 [dec/frmConf.frm](dec/frmConf.frm) 声明并使用 `SetLayeredWindowAttributes`、`UpdateLayeredWindow`、`AlphaBlend`。
- **窗口子类化与消息级控制**：在 [dec/mContext.bas](dec/mContext.bas)、[dec/mFunctions.bas](dec/mFunctions.bas) 多处 `SetWindowLong(..., GWL_WNDPROC, ...)`，说明通过 WndProc 精细控制交互与行为。
- **无边框窗体 + 自定义阴影窗体**：`frmMain/frmEdit/frmShadow` 均是 `BorderStyle = 0`，见 [dec/frmMain.frm](dec/frmMain.frm)、[dec/frmEdit.frm](dec/frmEdit.frm)、[dec/frmShadow.frm](dec/frmShadow.frm)。
- **拖拽与系统消息兼容处理**：`DragAcceptFiles`、`ChangeWindowMessageFilter` 出现在 [dec/mContext.bas](dec/mContext.bas)，说明对系统级消息做了兼容。
- **快捷方式解析是双路径策略**：在 [dec/Groups.ctl](dec/Groups.ctl) 可看到 `.lnk` 判断 + Win32/COM 处理 + `WScript.Shell` 回退。
- **启动动作偏原生简洁**：主要用 `ShellExecute` / `ShellExecuteExA`，见 [dec/mContext.bas](dec/mContext.bas)、[dec/Groups.ctl](dec/Groups.ctl)。
- **有主动内存工作集回收**：`SetProcessWorkingSetSize(..., -1, -1)` 见 [dec/mContext.bas](dec/mContext.bas)。
- **单线程模型**：项目配置 `MaxNumberOfThreads = 1`，见 [dec/Project.vbp](dec/Project.vbp)。

这说明旧版的“快”和“高密度”核心不在业务逻辑，而在：
- 直接系统绘制路径（GDI）
- 消息级窗口控制（WndProc/subclass）
- 窗口模型非常简单（单线程 + 轻界面）

---

## 3. 与当前 Tauri 架构的本质差异

### 3.1 你现在体感上的“慢/怪”来自哪里

不是 Rust 慢，而是 UI 运行时模型不同：

- Tauri UI 走 WebView（HTML/CSS 布局、重排、合成）；
- 多窗口 WebView + 无边框 + 自定义阴影，在 Windows 下更容易出现焦点、残影、虚框类问题；
- Web 风格默认 spacing 偏大，不天然等于“高密度工具 UI”。

### 3.2 哪些需求对 Tauri 不友好

- 你要“长期极致轻量占用 + 原生高密度 + 深度窗口控制”；
- 你明确不喜欢 Web 样式语义；
- 你在意的是桌面工具质感，而不是跨平台开发效率。

这三条叠加时，Tauri 通常不是最优终态。

---

## 4. 是否换栈：决策建议

## 4.1 不建议今天立刻全量重写的原因

- 当前 Rust 业务层（数据兼容、图标、启动项逻辑）已经可用；
- 你遇到的问题集中在窗口/UI 层，不是所有层都要推倒；
- 直接切 Win32/duilib 会把交付风险一次性拉满。

## 4.2 建议的目标架构

### 路线 A（推荐）：Rust 核心 + Slint 前端

- 保留 Rust 后端模块；
- 用 Slint 重建 UI（单窗口、原生渲染、高密度布局）；
- 性能/占用和原生质感显著优于 WebView；
- 维护复杂度低于 Win32/duilib 全自绘。

### 路线 B（极致性能）：Win32/duilib

- 可做到最接近旧版甚至更强的窗口控制与性能上限；
- 但开发与维护成本最高，且未来迭代门槛高。

---

## 5. 推荐执行方案（你可以直接按这个推进）

## 阶段 1：Tauri 止损（1~2 周）

目标：快速验证是否还能满足你。

- 编辑窗改为主窗内模态（移除第二 WebView）；
- 减少/去掉复杂自定义窗口效果（先稳住残影虚框）；
- 做高密度样式（列表行高、字体、间距整体压缩）；
- 建立性能测量（冷启动、首屏、空闲内存、操作延迟）。

### 阶段 1 门槛（建议）

- 冷启动首屏 < 700ms
- 空闲内存 < 150MB
- 常规交互响应 < 50ms
- 无明显焦点/残影问题

若达不到，进入阶段 2。

## 阶段 2：迁移到 Slint（4~8 周）

- Rust 业务逻辑保持不变；
- UI 与窗口交互重写；
- 优先完成主窗、列表、编辑、拖拽、托盘、热键；
- 完成后再决定是否继续深挖到 Win32/duilib。

---

## 6. 回答你的核心问题

你问“到底继续 Tauri 还是换技术栈”。

基于旧版实现事实，我的答案是：

- **产品长期方向**：应换到原生 UI 栈（优先 Slint）。
- **工程落地节奏**：先做一轮 Tauri 止损，不要当天全量重写。

这样做能同时兼顾：
- 你要的性能和密度上限；
- 当前项目已有成果的复用价值；
- 迁移风险与交付节奏。

---

## 7. 你现在就能做的决策

如果你想“尽快进入正确路线”，建议今天定这三个决策：

1. 立项阶段 1（Tauri 止损）并限制周期 2 周；
2. 同步预研 Slint（先做一个主窗 POC 验证性能与密度）；
3. 2 周后按数据做 Go/No-Go，不再凭主观体感争论。

这会让你的技术路线从“感觉驱动”变成“指标驱动”，决策会更稳。