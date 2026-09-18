# uikit 重构计划：视觉预制层（主题 + SVG）

> 状态：**待批准**。本文件在 `restructure/theme-svg-presets` 分支上，未改动任何现有代码。
> 否决方式：删掉本分支即可，`main` 与保底分支 `legacy/qt-ant` 均不受影响。

## 背景与目标

uikit 原为 nassistant monorepo。`cpp-duilib-launcher`（→ mlaunch）、
`libicon-*`（→ micon）已迁出，仓库里剩下的 31MB 主要是 Qt 遗产，控件代码为零。

转型目标：uikit 成为**视觉统一的空间与主题的预制层**，按"共享程度"分三层：

| 层 | 内容 | 框架相关性 | 落点 |
|---|---|---|---|
| L1 令牌数据 | 色板 / 字体 / 间距 / 圆角 / 图标语义映射（json、xml） | 中立，天然共享 | `uikit/design/` |
| L2 主题引擎 | 令牌读取校验、状态色派生、明暗合并、DPI 计算，输出"已解析主题对象" | 中立，纯 C++17，**跨框架共享** | `uikit/core` 引擎库 |
| L3 应用器 | 主题对象 → duilib xml/SkinSwitch；→ QPalette/QSS | 每框架一份，极薄 | `uikit/duilib/`（qt 以后再说） |

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
├── duilib/          # L3：theme applier（读 L2 输出 → 写 duilib 属性）+ 皮肤资源
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

## 阶段四：消费闭环

1. `luiox-repo` 注册 `uikit` 包定义（与 libca / libmcp / micon 同款）
2. mlaunch：`add_requires("uikit", "micon")`，去掉本地主题实现的重复
   （mlaunch 若用 libca 是它自己的事，与主题链无关）
3. lab-duilib 增补一个"主题切换" demo，作为端到端验收

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

## 决策点

1. ~~引擎落点~~：**已定** —— 与 libca 无关，引擎放 `uikit/core`（2026-09-18）
2. Qt 线（libant-qt 的字体乱码、控件迁移等待做项）：确认放弃还是另立 legacy 仓继续？
3. 执行范围：四个阶段全做，还是先做阶段一、二（清壳+骨架），三、四另约时间？
