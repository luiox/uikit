# libant-qt-demo

`libant-qt-demo` 是从原 `QtAntDesign` 应用壳中拆出来的独立演示工程。

## 当前定位

- 演示 `libant-qt` 组件的组合效果
- 承载页面级示例：`HomePage`、`FunctionPage`、`SettingsPage`、`AboutPage`
- 作为资源回退和交互验证入口

## 当前拆分状态

已迁出：

- `main.cpp`
- `LibAntQtDemoWindow.*`
- `HomePage.*`
- `FunctionPage.*`
- `SettingsPage.*`
- `AboutPage.*`

说明：

- 当前 demo 仍直接复用 `../libant-qt/libant-qt.qrc`
- 首页已先改成轻量静态页，避免强依赖多媒体模块
- 其余 demo 页面后续还会继续做进一步清理和独立化

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
