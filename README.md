# nassistant

`nassistant` 是基于 **Rust + Tauri v2 + Vue 3 + TypeScript** 的轻量启动器 MVP，目标是替代旧 VB6 版本，并兼容历史数据。

## 当前状态

- 已完成 Tauri v2 迁移（配置、托盘 API、全局快捷键 API）。
- 已实现启动器 MVP 主流程：
  - 读取并兼容导入 `Data.json`
  - 解析并兼容部分 `Poner.cfg` 设置
  - 分组展示、搜索过滤、条目启动
  - 非模态编辑窗口（新增/修改/删除）
  - 托盘显示/隐藏/退出
  - 全局快捷键（默认 `Alt+1`）

## 技术栈

- Tauri: `2.x`
- Rust: `edition = 2021`（建议 `rustc >= 1.74`）
- 前端: `Vue 3 + TypeScript + Vite`

## 目录结构

```text
03_nassistant/
  ui/
    index.html
    editor.html
    src/
      MainApp.vue
      EditorApp.vue
      main.ts
      editor.ts
      api.ts
      types.ts
  src-tauri/
    src/main.rs
    tauri.conf.json
    Cargo.toml
    capabilities/default.json
  Data.json
  Poner.cfg
```

## 本地开发

### 1) 安装依赖

```bash
npm install
```

### 2) 启动开发模式

```bash
npm run tauri dev
```

> 说明：`tauri dev` 会自动执行 `beforeDevCommand`（即 `npm run ui:dev`），无需手动开两个终端。

### 3) 构建

```bash
npm run tauri build
```

> 说明：`tauri build` 会自动执行 `beforeBuildCommand`（即 `npm run ui:build`）。

### 4) 前端单独调试（可选）

```bash
npm run ui:dev
```

### 5) 前端单独构建（可选）

```bash
npm run ui:build
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

## 主要配置

- `tauri.conf.json` 使用 **v2 schema**（`https://schema.tauri.app/config/2`）
- 已启用能力文件：`src-tauri/capabilities/default.json`
- 前端已切换到 **官方 npm API**（`@tauri-apps/api`），不再依赖 `window.__TAURI__` 全局注入
- 托盘与全局快捷键通过 v2 API + 插件实现：
  - `tauri::tray::TrayIconBuilder`
  - `tauri-plugin-global-shortcut`

## 已知限制（MVP）

- 当前未实现 Skills/MCP/Prompt 管理（后续阶段）。
- 快捷键修改后，运行时会保存；如遇冲突，建议改为其他组合键后重启验证。
- 图标提取目前为简化模式，后续会做缓存与懒加载优化。

## 下一步建议

- 增加分组增删改与排序
- 增加启动失败详情弹窗（错误码/路径/参数）
- 增加图标缓存与性能优化
- 接入 Skills/MCP/Prompt 项目级配置（第二阶段）
