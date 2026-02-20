# VB6 反编译 UI 参数与需求分析（Poner）

> 目的：基于 `dec/*.frm`、`dec/*.ctl`、`dec/*.bas` 的反编译结果，提炼可落地的 UI 视觉参数与交互需求，作为当前 C++/DuiLib 版本的对齐标准。
>
> 说明：反编译文本含大量 `Unknown_VTable_Call` 与局部编码乱码，本文将参数分为 **确定值**（可直接读取）与 **推断值**（需运行态验证）。

---

## 1. 证据来源与可信度

### 1.1 主要文件
- `dec/frmMain.frm`：主窗口定义（标题、边框、主控件装配）
- `dec/UI.ctl`：顶部条（菜单/搜索/退出）与主 UI 颜色入口
- `dec/Tabs.ctl`：分组（Tab）管理与分组右键菜单
- `dec/Groups.ctl`：项目列表、项目右键菜单、列表颜色入口
- `dec/Search.ctl`：搜索输入与搜索结果列表颜色入口
- `dec/frmEdit.frm`：项目编辑窗口（尺寸、控件、按钮）
- `dec/frmConf.frm`：设置窗口（多面板布局、整体色调）
- `dec/frmShadow.frm`：阴影/拖动辅助窗口

### 1.2 可信度分级
- **A（高）**：`.frm/.ctl` 顶部属性区（`Width/Height/BackColor/Font/BorderStyle`）
- **B（中）**：事件中明确常量（例如 `RGB(230,230,230)`）
- **C（低）**：`Unknown_VTable_Call` 周边行为推断（需运行时核验）

---

## 2. 全局视觉基线（可直接用于实现）

## 2.1 字体与风格
- 全局字体：微软雅黑（`Name="微软雅黑"` 或同义编码）
- 常见字号：9.75（主界面），9（编辑/设置子控件）
- 风格：扁平、无圆角、弱对比灰阶、轻边线

## 2.2 颜色基线（确定值）
- 顶部主背景：`RGB(230,230,230)`（`UI.ctl`）
- 主面板背景：`RGB(255,255,255)`（`Groups/Tabs/frmEdit/frmConf`）
- 主文字色：`RGB(90,90,90)`（`Groups/frmEdit/frmConf/Search`）
- 搜索容器背景：`RGB(210,210,210)`（`Search.ctl`）
- 分隔线/辅助线：`RGB(230,230,230)`（`frmConf` 线绘制）

## 2.3 窗口形态
- 多窗体 `BorderStyle = 0`（无系统边框）
- 多窗体 `ShowInTaskbar = 0`（不在任务栏）
- 主窗/编辑窗/设置窗均为自绘框架语义

---

## 3. 主界面（frmMain + UI + Tabs + Groups + Search）

## 3.1 主窗体参数（frmMain）
- 标题：`Poner`
- `BorderStyle = None`
- `ClientWidth = 6930`，`ClientHeight = 3600`
- `KeyPreview = True`
- 承载控件：`UI`、`Tabs`、`Groups`、`Search`

### 3.2 顶栏（UI.ctl）需求
- 背景色：`RGB(230,230,230)`（事件初始化时设置）
- 包含按钮语义：搜索、菜单、退出（`btnSrch`、`btnMenu`、`btnQuit`）
- 主菜单项（需功能对齐）：
  - 新建项目（含自定义/空项目/计算机/控制面板/回收站/注销/关机/重启）
  - 排序、自动隐藏锁定、路径转换、导入导出、检查更新、设置、官网、退出

### 3.3 分组条（Tabs.ctl）需求
- 背景色：`RGB(255,255,255)`
- 分组菜单：新建分组、重命名、删除
- 分组重命名确认键：`Enter`（`Text1_KeyPress` 触发 LostFocus 逻辑）

### 3.4 项目列表（Groups.ctl）需求
- 背景色：`RGB(255,255,255)`
- 前景色：`RGB(90,90,90)`
- 项目右键菜单：
  - 管理员运行
  - 打开文件位置
  - 资源管理器菜单
  - 复制完整路径
  - 移动到分组
  - 编辑
  - 删除

### 3.5 搜索区（Search.ctl）需求
- 搜索容器色：
  - BG：`RGB(210,210,210)`
  - FG：`RGB(90,90,90)`
- 交互特征：
  - `Text1_Change` 实时解析关键词并刷新结果
  - `Enter` 触发执行
  - 内置命令语义：运行、cmd、百度搜索、setting、shutdown/reboot/logoff/screenoff（字符串证据明确）

---

## 4. 子窗口规范

## 4.1 编辑窗口（frmEdit）
- `BorderStyle=None`，`StartUpPosition=CenterScreen`
- 尺寸：`ClientWidth=5070`，`ClientHeight=5775`
- `Form_Load` 中明确：
  - `BackColor = RGB(255,255,255)`
  - `ForeColor = RGB(90,90,90)`
- 核心控件：`Title`、`Path`、`Argument`、`IconLocation`、`CMDOk`、`CMDCancel`
- 图标预览：`Picture1` + `SHGetFileInfo` / `DrawIconEx`

## 4.2 设置窗口（frmConf）
- `BorderStyle=None`，`StartUpPosition=CenterScreen`
- 尺寸：`ClientWidth=12255`，`ClientHeight=7695`
- `Form_Load` 中明确：
  - `BackColor = RGB(255,255,255)`
  - `ForeColor = RGB(90,90,90)`
- 多 `PNL(index)` 面板切换结构
- 存在自绘线：`RGB(230,230,230)`（用于分割/边界）

## 4.3 阴影窗（frmShadow）
- 隐藏、无边框、不进任务栏
- 用于辅助拖动/阴影行为（存在鼠标捕获与 `SendMessage` 调用）

---

## 5. 当前实现应满足的“需求清单”

## 5.1 必须项（MUST）
1. 无系统标题栏，自绘顶部栏与关闭/菜单/搜索入口。
2. 全局颜色遵循灰阶基线：
   - 顶栏 `230,230,230`
   - 内容白底 `255,255,255`
   - 主文字 `90,90,90`
   - 搜索背景 `210,210,210`
3. 分组与项目均支持右键菜单，菜单项语义不缺失。
4. 分组支持新建、重命名、删除，重命名可 `Enter` 确认。
5. 搜索支持实时过滤与 `Enter` 执行。
6. 编辑窗口与设置窗口使用同一视觉语言（白底+灰字+扁平按钮）。

## 5.2 建议项（SHOULD）
1. 所有按钮/列表项 hover 与 pressed 仅做轻微灰阶变化，不引入强调色。
2. 所有边线保持 1px 级别浅灰线。
3. 保持“左分组 + 右项目 + 顶栏 + 状态区”的经典分区结构。

## 5.3 待核验项（TODO，运行态确认）
1. 各区精确高度（顶栏/状态栏/行高）在反编译中分散，建议以截图对齐进行二次标定。
2. 某些乱码 Caption 需按 GBK/CP936 原文还原后再做最终文案对齐。
3. 阴影窗与自动隐藏动画的像素级行为需要录屏逐帧比对。

---

## 6. 参数映射建议（给 DuiLib）

- `BG_MAIN = #FFFFFF`（内容）
- `BG_TOP = #E6E6E6`（顶部）
- `BG_SEARCH = #D2D2D2`（搜索区）
- `FG_TEXT = #5A5A5A`（主文本）
- `LINE = #E6E6E6`（线/边框）

> 说明：以上映射全部来自反编译常量，属于“可直接执行”的第一版视觉 token。

---

## 7. 交付结论

- 已能明确提炼出一套 **稳定且可实现** 的 VB6 风格参数：颜色、字体、边框形态、主交互菜单。
- 下一阶段可基于本文直接做“像素级收敛”：逐项对齐顶栏高度、分组行高、状态栏留白与 hover 对比度。
