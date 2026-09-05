# uikit

duilib 控件库（孵化期，**准入制**）。

本仓库正在转型：从 nassistant monorepo 收缩为单一的 duilib 控件层，
目标是让 duilib 用起来不费劲——但**不做预先设计**，控件只有被证明可复用后才能进入。

## 准入法则：三拷贝

1. **第一份拷贝：住在 app 里**（如 mlaunch 的 `src/ui/ui_controls.*`）。
   它只是该项目的实现细节，不抽象、不提前入库。
2. **第二份拷贝：在 lab 里以 demo 形式复制并抽象**
   （[lab-duilib](https://github.com/luiox/lab-duilib)），剥离业务依赖，
   验证"去掉业务之后它还成立吗"。大多数控件死在这一步——死掉是正常的。
3. **提炼进本仓库**：lab 验证通过 + 通过下方准入清单，才允许落库；
   之后 mlaunch / lab 改为 submodule 引用本仓库。

## 准入清单

- [ ] **第二消费者**：一个 lab demo + 一个真实 app 场景同时在用；只有一个场景不得入库。
- [ ] **fork 缺口说明**：补的是什么缺口（状态色/皮肤资源/DPI/SVG…）；
      duilib 原生或 fork 已解决的，直接用原生，不包装。
- [ ] **不重包原则**：薄层实现，不隐藏 duilib 原生 API，
      不出现"必须经过 uikit 才能用 duilib"的设计。
- [ ] **复合优先**：能靠 XML 属性/控件组合解决的，不写 C++ 子类。
- [ ] **文档**：用法、XML 属性示例、已知坑，一篇 md 跟代码走。

## 当前状态

- **已入库控件：无。** 等待 lab-duilib 孵化出第一批候选
  （预期来源：mlaunch 已验证的 ButtonUI/CheckBoxUI/IconButtonUI/Theme、
  dpi_helper、file_icon_control 等，逐个按清单审查，不整体迁移）。
- 计划中的控件方向参考《未来可能要做的控件列表.md》。
  注意：其中 Window/Panel/Button/Edit/ListBox 等 duilib 原生已有，
  不要照单重造；真正的缺口是 fork 补丁与复合控件。

## 历史遗留（渐进式拆走，冻结不动）

本仓库原为 nassistant monorepo，以下目录冻结保留、逐步拆走：

| 目录 | 去向 |
|---|---|
| `cpp-duilib-launcher/` | 已迁移至独立仓库 [mlaunch](https://github.com/luiox/mlaunch)（提交 ca18895 标记） |
| `libicon-core/`、`libicon-qt/` | 已提炼为独立仓库 [micon](https://github.com/luiox/micon) |
| `libant-qt/`、`libant-qt-demo/`、`libant-qt-demos/` | Qt 遗产，与 duilib 控件库无关，待决定去留 |
| `ant_demo/`、`qt6_widget_demo/`、`libicon-qt` 相关 demo | 同上，Qt 遗产 |
