# uikit 重构计划：视觉预制层（主题 + 统一控件 + 无边框窗口）

> 状态：**待批准**。本文件在 `restructure/theme-svg-presets` 分支上，未改动任何现有代码。
> 否决方式：删掉本分支即可，`main` 与保底分支 `legacy/qt-ant` 均不受影响。

## 背景与目标

uikit 原为 nassistant monorepo。`cpp-duilib-launcher`（→ mlaunch）、
`libicon-*`（→ micon）已迁出，仓库里剩下的 31MB 主要是 Qt 遗产，控件代码为零。

转型目标：uikit 解决**界面库（duilib fork）不解决的三件事**——
主题、一套统一风格的常用控件、无边框窗口预制件，并整体作为
luiox-repo 包对外（包装约束是本计划的一等公民，见「包装设计」节）。
按"共享程度"分三层：

| 层 | 内容 | 框架相关性 | 落点 |
|---|---|---|---|
| L1 令牌数据 | 色板 / 字体 / 间距 / 圆角 / 图标语义映射（json、xml） | 中立，天然共享 | `uikit/design/` |
| L2 主题引擎 | 令牌读取校验、状态色派生、明暗合并、DPI 计算，输出"已解析主题对象" | 中立，纯 C++17，**跨框架共享** | `uikit/core` 引擎库 |
| L3 应用器与控件 | 主题对象 → duilib xml/SkinSwitch；统一风格控件；无边框窗口预制件；→ QPalette/QSS | 每框架一份，极薄 | `uikit/duilib/`（qt 以后再说） |

设计纪律：**L1 令牌格式中禁止出现 duilib 属性语法**（如 `font="宋体;12"`），
防止 L3 反向污染 L1，保住共享。

已拍板：引擎**与 libca 无关**，放 `uikit/core`。libca 的边界是系统级通用工具
（str/fs/json/http…），theme 带视觉设计的领域语义，不进 libca；数据、引擎、
duilib 翻译层自包含在 uikit 一个仓里，对外只发一个 `uikit` 包。
为保留将来的拆分余地：`core` 不 include 任何 `duilib/` 的头，框架无关性靠
编译边界钉死，真出现非 uikit 消费者时整目录平移即可成库。

## 阶段一：uikit 清壳（纯删除 + 重写说明文档）

1. 删除已迁出目录（mlaunch / micon 已有完整实体，git 历史亦可找回）：
   `cpp-duilib-launcher/`、`libicon-core/`、`libicon-qt/`
2. 删除 Qt 遗产（**保底分支 `legacy/qt-ant` 已指向含全部文件的 main**，随时可整分支找回）：
   `libant-qt/`、`libant-qt-demo/`、`libant-qt-demos/`、`ant_demo/`、`qt6_widget_demo/`
3. 删除 Qt 线文档（legacy 分支保留）：
   `待做列表.md`、`VB6_UI参数与需求分析.md`、`C++实现_vs_VB6标准_逐项差异表.md`、`icon库列表.md`
4. 保留：`third_party/DuiLib_DuiEditor` submodule（皮肤编辑工具链）、
   `未来可能要做的控件列表.md`（按 README 警告裁剪，不照单施工）
5. 重写根 `xmake.lua`：8 个 `includes()` 全删，project 改名 `uikit`，
   暂不含任何 target（L3 有了再建）

## 阶段二：新结构落地（骨架 + 第一批资产）

```
uikit/
├── design/          # L1：tokens.json（色板/字体/间距）+ icons.map.json（语义名→micon图标）
├── core/            # L2：主题引擎（纯 C++17，禁止依赖 duilib/Qt 任何头文件）
├── duilib/          # L3，三个子目录：
│   ├── applier/     #   主题对象 → duilib 属性/xml（读 L2 输出）
│   ├── controls/    #   统一风格常用控件（ButtonUI/CheckBoxUI/SearchBoxUI…，按准入制逐个进）
│   └── frameless/   #   无边框窗口预制件（见「无边框边界」）
├── docs/            # 每篇主题：设计意图 + XML 用法 + 已知坑
└── third_party/     # DuiLib_DuiEditor（不动）
```

1. `design/` 第一版：从 mlaunch `src/ui/` 的 Theme 中**反向提炼**令牌（不发明新值，
   以已验证的真实配置为唯一来源）
2. `duilib/` 第一版：nanosvg 接线（参考 micon 的 `DuiLib_nanosvg_compat.md`）
   + applier 原型
3. README 重写：定位改为"视觉预制层"，**三拷贝准入制原样保留**（控件只有
   lab-duilib 验证通过 + 双消费者才入库；和主题绑定的控件优先）

## 阶段三：uikit/core 主题引擎（工作量主体）

1. 新建 `core/`（`include/uikit/theme/` + `src/` + `unittest/`），根 xmake.lua 建
   `uikit_core` 静态库 target；单元测试框架跟仓库走，规则简单自己定，不受 libca 模块规范约束
2. 功能范围：color（解析/混合/派生 hover·pressed·disabled）、token loader、
   明暗主题合并、dpi 缩放、resolved theme 数据结构；令牌解析实现可以依赖 libca/json（消费关系，不是入住）
3. 编译期纪律：core 的 include path 里不出现 duilib/Qt，框架无关性由构建系统保证
4. 验证即"第二消费者"法则：core 落地后，回改 mlaunch 的 Theme 消费它，
   编译 + 单测 + 手动跑通主题切换，才算成立

## 无边框边界（谁修什么）

基础无边框（WS_POPUP、caption 拖动、NCHITTEST 缩放）fork 原生已扛住——
mlaunch 在这块零自写代码，是现状的既成事实，不推翻。真正的缺口是：
窗口阴影、Win11 圆角与 Snap Layout、最大化遮挡任务栏、Per-Monitor DPI
缩放抖动、最小化动画。分工：

- **消息层修补**（WM_NCCALCSIZE / DWM 调用 / 消息循环行为）→ 归 DuiLib fork
  PR 流程，走 fork 的回归门；uikit 不私改消息语义。
- **组合层预制件** → `duilib/frameless/`：无边框窗口模板 + TitleBarUI +
  阴影/圆角配置的打包组合，走三拷贝准入制。它补的是"组合与默认值"缺口，
  不是重包原生（准入清单「fork 缺口说明」栏照常填写）。
- TitleBarUI 是首批候选里优先级最高的：它与主题（状态色）、图标（micon）、
  无边框（拖拽区/双击最大化）三方耦合，是三件事的交汇点。

## 包装设计（luiox-repo 准入，一等公民约束）

链路上真正的堵点是 DuiLib 依赖形态：libca / micon 自包含或只依赖 xrepo 包，
而 duilib 目前是"每个消费者自带 submodule 在树内编译"。若 uikit 包内自建
duilib，消费者进程会出现两份 DuiLib 静态库（全局单例 + ODR，必炸）。
因此依赖链自底向上成包：

1. **`duilib` 先成包**（前置条件）。fork 根 xmake.lua 已具备可包形态：
   `DuiLib` target 默认 static、`public` includedirs、UILIB_STATIC 处理、
   排除清单单一事实源、pugixml/nanosvg/stb 内部依赖齐备。luiox-repo 注册
   `duilib` 包（git 直连 fork，锁 commit，on_install 指定 `targets = "DuiLib"`）。
   落地时须核对：fork 排除口径与 mlaunch DuiLibLite 的差异项（Cairo/Gtk）
   以 fork 登记表为准对齐；on_test 用链接级自检。
2. **`uikit` 包一个入口拉齐整条链**。内部 target：
   - `uikit_core`（L2 静态库，纯 C++17，win/linux/mac 都能编）；
   - `uikit_duilib`（L3 静态库，`add_deps("duilib")` 公开传导，仅 windows）；
   - `design/` 数据文件随包安装（micon icons 同款）。
   配置面吸取 micon 教训（显式传 false 不传导）：`core_only` 默认 false
   （默认整包 = 绝大多数消费者的真实需求），需要纯引擎的消费者显式传 true。
3. **运行期数据不依赖包路径**。包安装目录在 app 运行期不可寻址，
   默认主题必须编译期内嵌（L1 json → 内建 resolved theme），外部 json
   仅作运行期换肤/覆盖入口；数据文件随包安装供工具链与文档使用。
4. **版本纪律沿用包仓既定做法**：add_versions 只追加不修改、锁 commit；
   第一个包版本发布前须有两消费者（准入制与包装准入同一条线）。

## 阶段四：消费闭环

1. 前置：`duilib` 包注册并验证（包装设计第 1 条）
2. `luiox-repo` 注册 `uikit` 包定义（与 libca / micon 同款）
3. mlaunch：`add_requires("uikit", "micon")`——删 DuiLibLite target、
   删最后一个 submodule（DuiLib_DuiEditor）、去本地 Theme/控件已入库部分的重复
4. lab-duilib 增补"主题切换"与"无边框窗口"demo，作为端到端验收（第二消费者）

## 明确不做

- ❌ 不写 `qt/` applier——当前无真实 Qt 消费者，只有 libant-qt 遗产；L1/L2 设计已为它留好位置
- ❌ 不把 `未来可能要做的控件列表.md` 照单实现——Window/Panel/Button 等 duilib 原生已有
- ❌ 不合并仓库——uikit（数据+引擎+翻译层，自包含）、micon（图标资产）两者边界即 L1/资产与图标层的边界
- ❌ 主题不进 libca——libca 保持系统级通用工具定位，theme 与它无关

## 风险与回退

| 风险 | 缓解 |
|---|---|
| 阶段一 31MB 删除看起来激进 | 每一类删除都有独立找回路径：legacy 分支 / mlaunch / micon / git 历史 |
| mlaunch 主题提炼工作量不可估 | 阶段一二不依赖阶段三，可先只清壳+立骨架，core 引擎单独立项 |
| 令牌格式设计闭门造车 | 强制从 mlaunch 反向提炼，不做预先设计；L1 schema 首版允许被阶段三推翻 |
| 引擎长在大仓里，将来别的项目想用不方便 | core 编译边界已钉死框架无关，届时整目录平移成独立库即可 |
| duilib 包与 mlaunch DuiLibLite 构建口径不一致 | 以 fork 登记表为准对齐排除清单；duilib 包 on_test 链接自检 + mlaunch 回改作为双验证 |
| fork 消息层修补（阴影/DPI）波及 mlaunch | 修补走 fork PR 流程不进 uikit；mlaunch 升级 duilib 包版本时按包仓版本纪律择机 |

## 决策点

1. ~~引擎落点~~：**已定** —— 与 libca 无关，引擎放 `uikit/core`（2026-09-18）
2. Qt 线（libant-qt 的字体乱码、控件迁移等待做项）：确认放弃还是另立 legacy 仓继续？
3. 执行范围：四个阶段全做，还是先做阶段一、二（清壳+骨架），三、四另约时间？
4. ~~uikit 能否进 luiox-repo~~：**已定** —— 能，且决定包装设计（见上）：duilib 先成包、uikit 依赖包一个入口拉齐、默认主题编译期内嵌、数据不依赖包路径（2026-09-18）
