# libant-qt-demos

`libant-qt-demos` 是面向 `libant-qt` 的分类化示例工程。

## 定位

- 提供按主题划分的控件示例，而不是单一场景型 showcase
- 补充 `libant-qt-demo` 之外的更完整控件组合样例
- 单独提供 `core/` 能力演示：`DesignSystem`、`IconProvider`、`GaussianBlur`、`StyleSheet`

## 当前页面

- `Core`：主题色板、图标获取、模糊预览、样式字符串预览
- `Common`：按钮、开关、搜索、下拉、数字输入、滑动条、组合表单
- `Feedback`：消息、进度条、骨架屏、暂无数据、状态徽章
- `Display`：玻璃卡片、Logo、数字动画、标签、二维码、分页

## 目录结构

```text
libant-qt-demos/
  xmake.lua
  README.md
  代码文档.md
  src/
    app/
      main.cpp
      DemoWindow.*
    pages/
      CoreCapabilitiesPage.*
      CommonControlsPage.*
      FeedbackShowcasePage.*
      DisplayShowcasePage.*
```

## 构建

在仓库根目录执行：

```bash
xmake f -c --qt="D:/Qt/6.8.3/msvc2022_64" --vs=2022 -p windows -a x64
xmake build --shallow libant_qt_demos
xmake run libant_qt_demos
```

## 与 libant-qt-demo 的区别

- `libant-qt-demo`：保留场景型页面和 showcase 组件联动
- `libant-qt-demos`：按控件类别与基础能力拆分，更适合作为开发参考手册