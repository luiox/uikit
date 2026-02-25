# ant_demo

这是一个演示如何用 DuiLib 实现 Ant Design 蓝白皮的最简项目。

## 目录结构

```
ant_demo/
  xmake.lua        -- 构建脚本
  src/
    main.cpp       -- 演示窗口
  skin/
    ant_skin.xml   -- 皮肤定义（颜色、样式、按钮等）
    assets/        -- PNG 图集和其他资源
  scripts/
    svg2png.py     -- 将 SVG 批量生成 PNG 的工具
```

## 构建与运行

1. 首次克隆后同步子模块：

   ```bash
   git submodule update --init --recursive
   ```

2. 使用 **uv** 初始化并管理依赖：

   在目录下执行：
   ```bash
   uv init          # 生成 pyproject.toml
   uv add cairosvg   # 添加转换所需依赖
   ```

   uv 会自动创建 `.venv` 并生成 `uv.lock`。以后只需
   ```bash
   uv sync          # 安装/更新依赖
   ```

   然后在运行应用之前，通过脚本把 SVG 转成 PNG：
   ```bash
   uv run scripts/svg2png.py path/to/svg-icons skin/assets
   ```

   该流程无需手动编辑 toml，全部由 uv 命令处理。


2. 添加或替换 `skin/assets` 中的 PNG 图集（可以用 `scripts/svg2png.py`
   从 SVG 自动生成）：

   ```bash
   python scripts/svg2png.py path/to/svg-icons skin/assets
   ```

3. 切换到项目目录并编译：

   ```bash
   cd ant_demo
   # 生成 PNG 资源（如果还没做）
   uv run scripts/svg2png.py svg-dir skin/assets

   xmake f -p windows -a x64 -m debug --toolchain=msvc
   xmake
   # 如果直接使用 `xmake run` 会先构建整个工作区，建议手动
   # 启动生成的可执行文件：
   cd build/windows/x64/debug
   ./ant_demo.exe
   ```

   程序会在与可执行文件同级的 `skin` 文件夹中查找 `ant_skin.xml`
   以及 `assets`，因此我们在构建步骤中自动将源码目录的 `skin`
   复制到输出。

   正常启动时应弹出含有按钮/单选/复选框的示例窗口。

## 按钮示例

- 普通按钮使用 `CButtonUI`，样式由 `style_ant_button` 定义。
- 图标按钮通过 `resourceid` 指向图集中的某个图块，并继承自
  普通样式。
- 单选/复选按钮由 `COptionUI` 实现，XML 中用 `group` 属性区分。

## 新皮肤生成流程

1. 用矢量工具设计需要的控件图标或背景，导出为 SVG。
2. 批量执行 `scripts/svg2png.py` 生成对应的 PNG 文件。
3. 在 `ant_skin.xml` 中编写 `style`，引用 `resourceid` 或直接设置
   `BkColor`、`BorderColor` 等属性。
4. 运行项目预览效果，可迭代调整。

---

以后若需要更多控件示例，可在 `skin/ant_skin.xml` 中增加，或在
`src/main.cpp` 中动态创建；本 demo 仅展示按钮相关用法。