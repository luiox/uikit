# libant-qt

`libant-qt` 是当前仓库里的 Qt Widgets 组件库层。

## 当前定位

- 提供可复用控件与基础交互能力
- 通过 `ant::IconProvider` 对接 `libicon-core` / `libicon-qt`
- 通用 SVG 图标统一收敛到 `libicon-core`
- 为独立 demo 工程 `libant-qt-demo` 提供组件支持
- 为分类化示例工程 `libant-qt-demos` 提供控件与 core 能力支持

## 当前结构

- 组件与基础设施源码：`src/`
- 核心基础设施：`src/core/`
- 输入控件：`src/input/`
- 视图控件：`src/view/`
- 布局与导航：`src/layout/`、`src/navigation/`
- 弹层与遮罩：`src/overlay/`
- 反馈与展示组件：`src/feedback/`、`src/display/`
- 图标语义入口：`src/core/IconProvider.*`
- 主题系统：`src/core/DesignSystem.*`
- 第三方源码 target：`third_party/`
- 分类化示例工程：`../libant-qt-demos/`

## 已完成的重构方向

1. 将项目名统一为 `libant-qt`
2. 切换到 xmake 作为正式构建入口
3. 接入 `libicon-core` + `libicon-qt`
4. 为一批核心类加上 `ant` 命名空间
5. 把 demo 主窗口与页面开始迁移到独立的 `libant-qt-demo`
6. 将 `src/` 内组件按职责分目录整理
7. 将登录弹窗、头像气泡、通知、抽屉等 demo 强耦合组件迁出到 `libant-qt-demo`
8. 新增 `libant-qt-demos` 作为控件分类示例与 core 能力演示工程

## 图标架构

统一入口在：

- `IconProvider.h`
- `IconProvider.cpp`

职责：

- `ant::IconRole -> QIcon`
- `ant::IconRole -> QPixmap`
- `ant::IconRole -> SVG 数据`
- 优先走 `libicon-core` / `libicon-qt`
- 通用角色优先走 `libicon-core` 的 `ANT_*` 枚举

## 构建

在仓库根目录执行：

```bash
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake build --shallow libant_qt
```

## 规划文档

- 组件库优化与发展路线：`优化与发展规划.md`

## 命名约束

- 库内公开类型统一定义在 `ant` 命名空间中
- 新增代码应直接使用 `ant::TypeName`
- 不再继续扩散全局命名空间兼容写法
