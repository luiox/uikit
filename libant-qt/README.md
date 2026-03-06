# libant-qt

`libant-qt` 是从原 `QtAntDesign` 中拆出来的 Qt Widgets 组件库层。

## 当前定位

- 提供可复用控件与基础交互能力
- 通过 `ant::IconProvider` 对接 `libicon-core` / `libicon-qt`
- 逐步减少对 `:/Imgs/...` 的直接耦合
- 为独立 demo 工程 `libant-qt-demo` 提供组件支持

## 当前结构

- 组件与基础设施源码：当前目录
- 图标语义入口：`IconProvider.*`
- 主题系统：`DesignSystem.*`
- 资源回退：`libant-qt.qrc`
- 第三方源码：`ThirdParty/`

## 已完成的重构方向

1. 将项目名从 `QtAntDesign` 调整为 `libant-qt`
2. 切换到 xmake 作为正式构建入口
3. 接入 `libicon-core` + `libicon-qt`
4. 为一批核心类加上 `ant` 命名空间
5. 把 demo 主窗口与页面开始迁移到独立的 `libant-qt-demo`

## 图标架构

统一入口在：

- `IconProvider.h`
- `IconProvider.cpp`

职责：

- `ant::IconRole -> QIcon`
- `ant::IconRole -> QPixmap`
- `ant::IconRole -> SVG 数据`
- 优先走 `libicon-core` / `libicon-qt`
- 未覆盖角色回退到旧 `qrc`

## 构建

在仓库根目录执行：

```bash
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake build --shallow libant_qt
```

## 后续拆分计划

当前仍有一部分偏 demo 的控件和页面残留在库目录内，后续会继续迁往 `libant-qt-demo` 或更细的示例模块。
