# QtAntDesign 重构计划与 xmake Qt6 方案

## 1. 目标

本次重构的核心目标不是“把所有 `:/Imgs/...` 字符串替换掉”，而是先建立一个 **可复用、可维护、可迁移** 的资源访问层，为后续把 `QtAntDesign` 变成真正的组件库打基础。

目标分为三层：

1. **统一图标访问入口**：优先接入 `icon-lib`，避免业务代码直接依赖 `:/Imgs/...`。
2. **把组件和 demo 分离**：先保住核心控件，再逐步剥离演示页面和示例素材。
3. **建立跨项目可复用的资源规范**：Qt、DuiLib、后续其他 UI 工程都尽量复用同一套图标语义和生成资产。

---

## 2. 当前问题归纳

### 2.1 项目结构问题

`QtAntDesign` 当前更像一个“带大量示例页的 Qt6 Widgets 应用”，而不是边界清晰的组件库：

- 核心控件和示例页混在一个工程里。
- 构建目标是可执行程序，不是独立库。
- `FunctionPage` 中混有大量 demo 资源、假数据、品牌素材、示例交互。

### 2.2 资源问题

当前资源依赖方式存在以下问题：

- 资源路径直接散落在控件代码中。
- `DesignSystem` 直接返回路径字符串或 `QIcon("...”)`。
- `QSvgRenderer` / `QIcon` / `QPixmap` 的创建入口不统一。
- 深浅色主题图标通过多份文件名硬编码切换。
- 资源难以在多个项目之间共享。

### 2.3 维护问题

- `qrc` 资源过多，图标、字体、图片、视频职责混杂。
- 现有工程已有明显维护债务，例如个别资源路径与资源表不一致。
- 现有 API 偏“字符串驱动”，缺少类型安全和语义抽象。

---

## 3. 重构原则

### 3.1 先抽象，再替换

不要先做全量批量替换。应先建立新的图标访问层，再逐步把控件迁过去。

### 3.2 只先重构“库级资源”

第一阶段只处理真正属于组件库的图标，不急着处理：

- JPG 轮播图
- demo 插画
- 示例头像 PNG
- 视频
- 字体

### 3.3 保留过渡期 fallback

新接口允许优先从 `icon-lib` 取图标，如果没有覆盖，再回退到旧 `qrc`，保证迁移可以渐进完成。

---

## 4. 推荐重构路线

## Phase 1：建立 Qt 图标适配层（建议优先完成）

新增一层统一接口，例如：

- `AntIcon`
- `AntIconRole`
- `QtIconProvider`
- `QtSvgIconEngine`（可选）

职责：

- `iconlib::Icon -> QIcon`
- `iconlib::Icon -> QPixmap`
- `iconlib::Icon -> QByteArray / SVG 数据`
- 支持主题染色
- 支持 fallback 到旧 `:/Imgs/...`
- 做缓存，避免频繁重新解析 SVG

建议接口示意：

```cpp
namespace ant {

enum class IconRole {
    NavHome,
    NavHomeActive,
    NavSettings,
    MsgInfo,
    MsgWarning,
    MsgError,
    WindowMin,
    WindowMax,
    WindowRestore,
    WindowClose,
    Search,
    Back,
    ArrowDown,
    ArrowRight
};

QIcon icon(IconRole role, bool dark = false);
QPixmap pixmap(IconRole role, QSize size, bool dark = false);
QByteArray svg(IconRole role, bool dark = false);

}
```

这样业务侧以后不再写：

```cpp
QIcon(":/Imgs/home.svg")
```

而是写：

```cpp
ant::icon(ant::IconRole::NavHome)
```

---

## Phase 2：替换核心控件

优先迁移这些真正属于“组件库能力”的模块：

1. `DesignSystem`
2. `AntButton`
3. `CustomToolButton`
4. `AntInput`
5. `AntComboBox`
6. `AntMessage`
7. `AntTabWidget`
8. `AntTreeView`
9. `CircularAvatar`
10. `NoDataWidget`

目标是把核心控件从 `:/Imgs/...` 路径耦合中解开。

---

## Phase 3：拆分库与 demo

建议把 `QtAntDesign` 最终拆成类似结构：

```text
qt-ant-core/        -- 主题、图标、基础工具
qt-ant-widgets/     -- 可复用控件
qt-ant-demo/        -- 演示程序
icon-lib/           -- 统一图标资产
```

拆分后：

- `qt-ant-core` 依赖 `icon-lib`
- `qt-ant-widgets` 依赖 `qt-ant-core`
- `qt-ant-demo` 依赖前两者 + demo 资源

---

## 5. 工作量评估

### 5.1 MVP：图标访问层 + 少量核心替换

内容：

- 建立 Qt 图标适配层
- 接入 `icon-lib`
- 替换 `DesignSystem` 与少量公共控件
- 保留旧 `qrc` fallback

预计：**3 ~ 5 人日**

### 5.2 中等重构：核心控件全面迁移

内容：

- 核心控件全部改走统一图标接口
- 简化 `DesignSystem`
- 清理一批旧资源路径

预计：**7 ~ 12 人日**

### 5.3 完整重构：组件库化

内容：

- demo / library 拆分
- 构建系统整理
- 资源体系梳理
- 文档与示例补齐

预计：**15 ~ 25 人日**

---

## 6. 结论

建议重构，但不要推倒重来。

**推荐顺序：**

1. 先做 `icon-lib` 接入层
2. 再迁核心控件
3. 最后拆 demo 与库

这条路径成本可控，风险最低，且最符合“以后多个项目复用同一套资源”的目标。

---

# 7. 如何用 xmake 构建 Qt6 Widgets 项目

## 7.1 关键结论

如果你要用 xmake 构建 Qt6 Widgets 项目，**最重要的一点** 是：

> 传给 xmake 的应该是 Qt SDK 根目录，而不是 `include` 目录。

你当前给出的路径是：

```text
D:\Qt\6.8.3\msvc2022_64\include
```

实际应该使用：

```text
D:/Qt/6.8.3/msvc2022_64
```

原因：xmake 需要从 Qt 根目录自动发现：

- `include/`
- `lib/`
- `bin/`
- `plugins/`
- `moc/uic/rcc`

如果只给 `include`，xmake 无法完成完整 Qt 工具链检测。

---

## 7.2 最小可用配置

一个最小 Qt6 Widgets 项目的 `xmake.lua` 通常可以这样写：

```lua
set_project("qt6_widget_demo")
set_version("0.1.0")
set_xmakever("2.8.2")

add_rules("mode.debug", "mode.release")

 target("qt6_widget_demo")
    set_kind("binary")
    set_languages("cxx17")
    add_rules("qt.widgetapp")
    add_frameworks("QtCore", "QtGui", "QtWidgets")
    add_files("src/*.cpp")
    add_headerfiles("src/*.h")
```

说明：

- `qt.widgetapp`：启用 Qt Widgets 应用规则。
- `add_frameworks(...)`：声明所需 Qt 模块。
- `add_files("src/*.cpp")`：添加源文件。
- `add_headerfiles("src/*.h")`：让 IDE 更完整，也方便后续带 `Q_OBJECT` 的类被管理。

---

## 7.3 推荐配置命令

如果你在当前仓库根目录下直接构建，推荐执行：

```bash
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake build --shallow qt6_widget_demo
xmake run qt6_widget_demo
```

说明：

- `-c`：清除旧配置，避免缓存污染。
- `--qt=...`：指定 Qt SDK 根目录。
- `--vs=2022`：显式使用 VS2022 工具链。
- `-p windows -a x64`：明确平台和架构。
- `--shallow qt6_widget_demo`：只构建 demo 目标，避免当前工作区内其他 target 干扰验证。

如果你把 `qt6_widget_demo/` 单独拷出去作为独立项目，再进入该目录执行普通的 `xmake f && xmake build` 也可以。

---

## 7.4 常见问题与解决办法

### 问题 1：给了 `include` 路径但仍然检测失败

解决：把路径改成 Qt 根目录。

错误示例：

```text
D:\Qt\6.8.3\msvc2022_64\include
```

正确示例：

```text
D:/Qt/6.8.3/msvc2022_64
```

### 问题 2：`QWidget` / `QObject` 头文件找不到

通常是以下原因：

- 没走 `qt.widgetapp`
- 没写 `add_frameworks("QtCore", "QtGui", "QtWidgets")`
- Qt SDK 根目录配置错误
- 没清理旧配置缓存

排查命令：

```bash
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake -vD
```

### 问题 3：程序能编译，但运行时缺少 Qt 平台插件

如果是在开发机本地运行，通常问题不大；如果要拷贝到别的目录运行，需做部署。

可选方案：

1. 使用 Qt 自带 `windeployqt`
2. 或者在开发阶段先在 Qt 环境下运行

例如：

```text
D:/Qt/6.8.3/msvc2022_64/bin/windeployqt.exe <your_exe_path>
```

### 问题 4：MSVC 版本不匹配

你当前 Qt 是：

```text
msvc2022_64
```

因此 xmake 也应使用 VS2022 工具链，不要混用 MinGW 或较老的 MSVC。

---

## 7.5 推荐实践

1. Qt 路径始终传 SDK 根目录。
2. Qt Demo 单独放一个目录，先验证规则和工具链。
3. 先用纯代码 Widgets Demo 跑通，再逐步引入 `ui/qrc`。
4. 真正做库项目时，把 `icon-lib` 接入为独立子模块，不再到处散写 `:/Imgs/...`。

---

## 8. 当前仓库内新增的 Demo

本仓库已新增：

```text
qt6_widget_demo/
```

用于验证：

- xmake + Qt6 Widgets 构建链路
- VS2022 + Qt6.8.3 SDK 检测
- 一个最小可运行的 QWidget 应用

建议先在仓库根目录验证：

```bash
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake build --shallow qt6_widget_demo
xmake run qt6_widget_demo
```

如果你希望下一步继续推进，我建议直接做：

1. 给 `QtAntDesign` 加一个 `QtIconProvider`
2. 把 `DesignSystem` 先接到 `icon-lib`
3. 先迁移窗口按钮、导航按钮、消息提示图标
