# 启动器 MVP 开发文档（第一阶段）

## 1. 文档目标
本阶段只聚焦“启动器核心能力”，快速交付一个可替代旧版 VB6 日常使用的版本，确保：
1. 可读写并兼容现有 `Data.json`；
2. 可稳定启动程序（含参数）；
3. 支持基础编辑、托盘、快捷键、隐藏/呼出；
4. 中文/UTF-8 环境稳定无乱码。

> 本文档不覆盖 Skills/MCP/Prompt 管理；这些能力进入后续里程碑。

---

## 2. MVP 范围定义

### 2.1 In Scope（必须完成）
1. 分组与条目展示（左侧分组、右侧启动项列表）；
2. 条目启动（含 `Arguments` 传参）；
3. 条目增删改（独立非模态编辑窗口）；
4. `Data.json` 首次导入与后续持久化；
5. 托盘常驻（显示主窗口 / 退出）；
6. 全局快捷键（默认 `Alt+1`）呼出/隐藏主窗口；
7. 启动后自动隐藏（可配置，兼容 `ExecuteHide`）；
8. 基础搜索（按名称过滤当前分组）。

### 2.2 Out of Scope（本阶段不做）
1. Skills 库管理；
2. MCP 服务器管理；
3. Prompt 片段库；
4. 多工作区一键切换；
5. 云同步、账号系统、在线更新。

---

## 3. 用户场景与验收口径

### 3.1 核心用户场景
- 打开面板 -> 选择分组 -> 点击程序 -> 程序启动，面板按配置隐藏。
- 通过快捷键 `Alt+1` 任意时刻呼出面板。
- 编辑某条目（名称/路径/参数/图标）并保存，重启后仍生效。
- 旧 `Data.json` 可直接导入，中文名称和路径显示正常。

### 3.2 MVP 验收标准（必须全部满足）
1. 使用现有 `Data.json` 导入后，分组数/条目数一致；
2. 至少抽测 30 个条目，启动成功率 ≥ 95%；
3. 含中文路径、空格路径、带参数条目均可启动；
4. 全局快捷键可稳定工作（冲突时有提示）；
5. 编辑增删改后数据不丢失；
6. 托盘菜单可“显示/隐藏/退出”；
7. 1000+ 条目场景可用，无明显卡死。

---

## 4. 技术选型（MVP）

## 4.1 框架与语言
- Rust + Tauri 2.x
- 前端：原生 HTML/CSS/JavaScript（不引入前端框架）

### 4.2 关键依赖（建议）
- `tauri`
- `serde` + `serde_json`
- `tokio`
- `anyhow` / `thiserror`
- `dirs`
- 可选：`tauri-plugin-single-instance`、`tauri-plugin-global-shortcut`、`tauri-plugin-window-state`

### 4.3 安全与权限原则
- 最小权限原则（只开必要 capability）；
- 启动程序使用参数化命令，不拼接 shell 字符串；
- 数据写入采用原子替换，避免文件损坏。

---

## 5. 数据模型与兼容方案

### 5.1 旧数据输入（兼容源）
- 文件：`Data.json`
- 结构：`{ GroupName: LaunchItem[] }`
- 字段：`Name` / `TargetPath` / `IconLocation` / `Arguments` / `Count`

### 5.2 MVP 内部模型
```text
LauncherState
  groups: [Group]
  selected_group_id: string
  settings: Settings

Group
  id: string
  name: string
  order: i32
  items: [LaunchItem]

LaunchItem
  id: string
  item_type: "app" | "separator"
  name: string
  target_path: string
  icon_location: string
  arguments: string
  launch_count: u64
  enabled: bool

Settings
  hotkey: string
  execute_hide: bool
  current_group: string
```

### 5.3 导入规则
1. 首次启动检测同目录是否存在 `Data.json`；
2. 若存在，执行一次导入并生成 `launcher.v2.json`；
3. 空路径条目或 `----xxx----` 识别为 `separator`；
4. 导入后不改写原始 `Data.json`；
5. 后续默认读写 `launcher.v2.json`。

### 5.4 配置兼容（Poner.cfg）
- 迁移：`CurrentTab` / `ActivatedTab`、`ExecuteHide`；
- 忽略：`Handle`、不可解释字段；
- 无法解析时采用默认值并记录日志。

---

## 6. 功能设计（页面与交互）

### 6.1 主窗口
- 左侧：分组列表（可滚动）
- 右侧：条目列表（图标 + 名称）
- 顶部：搜索框（按名称过滤）
- 操作：双击/回车启动，右键打开编辑（可选）

### 6.2 编辑窗口（非模态）
- 字段：名称、目标、参数、图标
- 按钮：保存、取消
- 行为：
  - 窗口单例，重复打开时聚焦；
  - 保存后主窗口即时刷新；
  - 校验失败时提示（目标为空、路径不存在等）。

### 6.3 托盘与快捷键
- 托盘菜单：显示主窗口、隐藏主窗口、退出
- 全局快捷键：`Alt+1`（支持设置修改）
- 启动后隐藏：按 `execute_hide` 配置决定

---

## 7. 后端命令接口（IPC）

```text
load_launcher_data() -> LauncherState
save_launcher_data(state) -> Result
launch_item(item_id) -> LaunchResult
upsert_item(group_id, item) -> Result
delete_item(group_id, item_id) -> Result
list_groups() -> [Group]
set_current_group(group_id) -> Result
get_settings() -> Settings
update_settings(settings) -> Result
show_editor(item_id?) -> Result
```

错误返回统一格式：
```text
{ code: string, message: string, detail?: string }
```

---

## 8. 项目结构（MVP）

```text
src-tauri/
  src/
    main.rs
    app/
      commands.rs
      window.rs
      tray.rs
      hotkey.rs
    domain/
      model.rs
      validator.rs
    infra/
      storage.rs
      migrate.rs
      process.rs
      cfg_compat.rs
ui/
  index.html
  editor.html
  styles.css
  app.js
  editor.js
```

---

## 9. 开发计划（MVP）

### 9.1 里程碑
| 里程碑 | 内容 | 预计工期 |
|---|---|---:|
| M1 | 脚手架、主窗口、托盘、快捷键 | 2-3 天 |
| M2 | `Data.json` 导入与内部存储 | 2-3 天 |
| M3 | 列表展示、搜索、启动能力 | 2-3 天 |
| M4 | 编辑窗口增删改 + 持久化 | 2-3 天 |
| M5 | 回归测试、打包、迁移说明 | 2 天 |

**总计：10-14 天（单人）**

### 9.2 每日交付节奏建议
- 每天输出可运行版本；
- 每晚回归“导入 + 启动 + 保存”主链路；
- 每个里程碑末输出演示录屏。

---

## 10. 测试清单（MVP）

### 10.1 功能测试
1. 导入后分组/条目计数一致；
2. 启动 `.exe/.cmd/.bat/.lnk`（如支持）行为正确；
3. 带参数启动（如 Joplin 参数）正确；
4. separator 不可启动；
5. 编辑新增/修改/删除后重启数据仍在。

### 10.2 兼容测试
1. 中文名称、中文路径、特殊符号路径；
2. 含空格路径和参数引号；
3. 缺失图标、目标不存在路径容错提示。

### 10.3 稳定性测试
1. 连续启动 100 次不崩溃；
2. 异常退出后数据文件可恢复读取；
3. 快捷键反复呼出/隐藏窗口稳定。

---

## 11. 发布物与交付标准

### 11.1 交付物
1. 可安装包（Windows）
2. 默认配置与迁移说明
3. MVP 使用说明（快捷键、编辑、托盘）
4. 已知问题清单

### 11.2 发布门槛（Go/No-Go）
- 所有 P0 缺陷关闭；
- 核心链路通过率 100%（导入、启动、保存、呼出）；
- 回归测试通过率 ≥ 95%。

---

## 12. 第二阶段接口预留（不实现）
仅预留字段与目录，不实现业务：
- `skills_enabled: []`
- `mcp_enabled: []`
- `prompt_sets_enabled: []`

这样 MVP 不返工，可平滑进入 Skills/MCP/Prompt 阶段。
