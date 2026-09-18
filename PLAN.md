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
| L2 主题引擎 | 令牌读取校验、状态色派生、明暗合并、DPI 计算，输出"已解析主题对象" | 中立，纯 C++17，**跨框架共享** | `libca/theme` 新模块 |
| L3 应用器 | 主题对象 → duilib xml/SkinSwitch；→ QPalette/QSS | 每框架一份，极薄 | `uikit/duilib/`（qt 以后再说） |

设计纪律：**L1 令牌格式中禁止出现 duilib 属性语法**（如 `font="宋体;12"`），
防止 L3 反向污染 L1，保住共享。

引擎放 libca 而非 `uikit/core` 的理由：theme 引擎零 UI 依赖，符合 libca 模块风格，
可随 luiox-repo 注册发布，非 uikit 项目也能直接 `add_requires("libca")` 复用；
放 uikit 内则是将来必然要拆的第二颗种子。

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

## 阶段三：libca/theme 引擎模块（工作量主体）

1. 按 libca 模块惯例新建 `libca/theme/`（`src/libca/theme/` + unittest + xmake.lua 接入，
   遵循 doc/02_单元测试规范.md）
2. 功能范围：color（解析/混合/派生 hover·pressed·disabled）、token loader（依赖 libca/json）、
   明暗主题合并、dpi 缩放、resolved theme 数据结构
3. 验证即"第二消费者"法则：libca/theme 落地后，回改 mlaunch 的 Theme 消费它，
   编译 + 单测 + 手动跑通主题切换，才算模块成立

## 阶段四：消费闭环

1. `luiox-repo` 注册 `uikit` 包定义（与 libca / libmcp / micon 同款）
2. mlaunch：`add_requires("libca", "uikit", "micon")`，去掉本地主题实现的重复
3. lab-duilib 增补一个"主题切换" demo，作为端到端验收

## 明确不做

- ❌ 不写 `qt/` applier——当前无真实 Qt 消费者，只有 libant-qt 遗产；L1/L2 设计已为它留好位置
- ❌ 不把 `未来可能要做的控件列表.md` 照单实现——Window/Panel/Button 等 duilib 原生已有
- ❌ 不合并仓库——libca（引擎）、micon（图标）、uikit（预制）三者边界即三层边界

## 风险与回退

| 风险 | 缓解 |
|---|---|
| 阶段一 31MB 删除看起来激进 | 每一类删除都有独立找回路径：legacy 分支 / mlaunch / micon / git 历史 |
| mlaunch 主题提炼工作量不可估 | 阶段一二不依赖阶段三，可先只清壳+立骨架，libca/theme 单独立项 |
| 令牌格式设计闭门造车 | 强制从 mlaunch 反向提炼，不做预先设计；L1 schema 首版允许被阶段三推翻 |

## 决策点（批准时请一并拍板）

1. 引擎落点：`libca/theme`（本计划默认）还是 `uikit/core`？
2. Qt 线（libant-qt 的字体乱码、控件迁移等待做项）：确认放弃还是另立 legacy 仓继续？
3. 执行范围：四个阶段全做，还是先做阶段一、二（清壳+骨架），三、四另约时间？
