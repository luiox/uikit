# qt6_widget_demo

这是一个最小可运行的 Qt6 Widgets + xmake 示例。

## 目标

用于验证：

- `xmake` 能否正确检测 Qt6.8.3
- `qt.widgetapp` 规则是否可用
- VS2022 + Qt6 `msvc2022_64` 工具链是否匹配

## 重要说明

你的 Qt 路径不要传到 `include`，而要传到 Qt SDK 根目录。

错误：

```text
D:\Qt\6.8.3\msvc2022_64\include
```

正确：

```text
D:/Qt/6.8.3/msvc2022_64
```

## 构建命令

### 方式 A：在当前仓库根目录构建（推荐）

```bash
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake build --shallow qt6_widget_demo
xmake run qt6_widget_demo
```

说明：当前仓库根目录已经通过 [xmake.lua](../xmake.lua) 注册了 `qt6_widget_demo` 子项目。
使用 `--shallow` 可以避免构建其他不相关 target。

### 方式 B：把本目录单独拷出去后构建

如果你把 `qt6_widget_demo/` 复制到一个独立目录，它也可以单独作为一个 xmake Qt Widgets 项目使用：

```bash
cd qt6_widget_demo
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake build
xmake run qt6_widget_demo
```

## 目录结构

```text
qt6_widget_demo/
  xmake.lua
  src/
    main.cpp
    MainWindow.h
    MainWindow.cpp
```
