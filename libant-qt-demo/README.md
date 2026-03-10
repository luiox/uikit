# libant-qt-demo

`libant-qt-demo` 是 `libant-qt` 的独立演示工程。

## 当前定位

- 演示 `libant-qt` 组件的组合效果
- 承载页面级示例：`HomePage`、`FunctionPage`、`SettingsPage`、`AboutPage`
- 作为资源回退和交互验证入口

## 当前拆分状态

已迁出：

- `src/app/main.cpp`
- `src/app/LibAntQtDemoWindow.*`
- `src/pages/HomePage.*`
- `src/pages/FunctionPage.*`
- `src/pages/SettingsPage.*`
- `src/pages/AboutPage.*`
- `src/showcase/CircularAvatar.*`
- `src/showcase/MaterialDialog.*`
- `src/showcase/DialogViewController.*`
- `src/showcase/LoginPageWidget.*`
- `src/showcase/RegisterPageWidget.*`
- `src/showcase/ProfilePage.*`
- `src/showcase/StandardDialogPage.*`
- `src/showcase/BubbleViewController.*`
- `src/showcase/UserInfoBubble.*`
- `src/showcase/DrawerWidget.*`
- `src/showcase/NotificationWidget.*`
- `src/showcase/NotificationManager.*`

说明：

- demo 资源已迁到 `assets/`，并由独立的 `assets/libant-qt-demo.qrc` 管理
- 页面级代码和展示型场景组件已按 `app / pages / showcase` 分类
- 首页已先改成轻量静态页，避免强依赖多媒体模块
- `assets/images/` 中当前包含可替换的本地占位图，用于保证 demo 构建和交互展示闭环

## 构建

在仓库根目录执行：

```bash
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake build --shallow libant_qt_demo
xmake run libant_qt_demo
```

## 设计约束

- demo 工程只负责页面装配和交互展示
- 通用控件、主题、图标适配应尽量留在 `libant-qt`
- 新增页面时，优先通过 `ant::IconProvider` 获取图标，不再直接扩散资源路径字符串
