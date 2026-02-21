# nassistant

`nassistant` 已迁移为 **C++17 + DuiLib + xmake** 的 Windows 启动器实现，目标是替代旧 VB6 版本，并兼容历史数据。

## 当前状态

- 主程序位于 `cpp-duilib-launcher/`，可直接编译运行。
- 已实现：
  - 读取并兼容导入 `Data.json`
  - 解析并兼容部分 `Poner.cfg` 设置
  - 分组/启动项渲染与右键菜单（添加/编辑/移动/删除）
  - 启动项双击启动、快捷键 `Enter/Delete`
  - 文件拖拽导入（含 `.lnk` 解析）
  - 搜索模式（跨分组过滤）

## 技术栈

- C++: `C++17`
- UI: `DuiLib`
- 构建: `xmake`
- 测试: `gtest`

## 目录结构

```text
03_nassistant/
  cpp-duilib-launcher/
    src/
    tests/
    xmake.lua
  Data.json
  Poner.cfg
```

## 本地开发

在第一次克隆项目后，请同步子模块：

```bash
git submodule update --init --recursive
```

然后进入主程序目录构建运行：

```bash
cd cpp-duilib-launcher
xmake -r
xmake run nassistant-duilib
```

## 运行测试

```bash
cd cpp-duilib-launcher
xmake run backend_tests
```

## 数据兼容说明

- 首次启动时，程序会在可访问路径中查找旧版 `Data.json`。
- 导入后将写入新的数据文件：
  - `launcher.v2.json`
  - `nassistant.settings.json`
- 默认存储目录：
  - Windows: `%LOCALAPPDATA%/nassistant/`
- 导入逻辑保留原始 `Data.json`，不会覆盖旧文件。

### 分隔项识别规则

满足任一条件时会识别为分隔项（不可启动）：
- `TargetPath` 与 `IconLocation` 都为空
- 名称格式为 `----xxx----`

## 说明

- 原 Tauri + Vue 实现（`ui/`、`src-tauri/`）已按迁移计划移除。
