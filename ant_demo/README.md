# ant_demo

这是一个最小可运行的 DuiLib 演示工程，用来保留仓库里的原生 DuiLib 皮肤加载链路。

## 目录结构

```
ant_demo/
  xmake.lua        -- 构建脚本
   skin/
      ant_skin.xml   -- 最小可运行皮肤
  src/
    main.cpp       -- 演示窗口
  scripts/
    svg2png.py     -- 将 SVG 批量生成 PNG 的工具
```

当前 `skin/assets/` 目录保留为空目录位，用于后续继续扩展 PNG 图集资源。

## 构建与运行

1. 首次克隆后同步子模块：

   ```bash
   git submodule update --init --recursive
   ```

2. 如果需要继续生成 PNG 资源，可使用 **uv** 初始化并管理依赖：

   在目录下执行：
   ```bash
   uv init          # 生成 pyproject.toml
   uv add cairosvg   # 添加转换所需依赖
   ```

   uv 会自动创建 `.venv` 并生成 `uv.lock`。以后只需
   ```bash
   uv sync          # 安装/更新依赖
   ```

   然后通过脚本把 SVG 转成 PNG：
   ```bash
   uv run scripts/svg2png.py path/to/svg-icons skin/assets
   ```

   该流程无需手动编辑 toml，全部由 uv 命令处理。

3. 如果后续添加图集资源，可将生成结果放进 `skin/assets/`：

   ```bash
   python scripts/svg2png.py path/to/svg-icons skin/assets
   ```

4. 切换到项目目录并编译：

   ```bash
   cd ant_demo
   xmake f -p windows -a x64 -m debug --toolchain=msvc
   xmake
   xmake run ant_demo
   ```

   程序会在与可执行文件同级的 `skin` 文件夹中查找 `ant_skin.xml`。
   构建脚本现在会自动复制源码目录下的 `skin/`，因此不会再因为
   缺失目录导致 `xmake run ant_demo` 失败。

   正常启动时应弹出包含按钮、单选与复选示例的最小窗口。

## 按钮示例

- 普通按钮使用 `CButtonUI`，样式由 `style_ant_button` 定义。
- 当前强调按钮直接复用主按钮样式变体，不再依赖必须存在的图集资源。
- 单选/复选按钮由 `COptionUI` 实现，XML 中用 `group` 属性区分。

## 新皮肤生成流程

1. 用矢量工具设计需要的控件图标或背景，导出为 SVG。
2. 批量执行 `scripts/svg2png.py` 生成对应的 PNG 文件。
3. 在 `ant_skin.xml` 中编写 `style`，引用 `resourceid` 或直接设置
   `BkColor`、`BorderColor` 等属性。
4. 运行项目预览效果，可迭代调整。

---

以后若需要更多控件示例，可继续扩充 `skin/ant_skin.xml` 或改为在
`src/main.cpp` 中动态创建；当前版本的目标是保证 DuiLib 皮肤运行链路稳定可回归。