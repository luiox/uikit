# VS Code profile 拆分策略（shared settings/keybindings + per-profile extensions）

概览
- Settings 与 Keybindings：仓库只存 `settings/common.json` + 可选的 `settings/<profile>.json`、`keybindings/common.json`。
- 扩展（extensions）：按 profile 分文件，公共项抽到 `extensions/common.txt`。
- 脚本：一键导出/导入/合并/去重（跨平台 Python CLI，适用于 Windows/macOS/Linux）

快速开始（在仓库根目录运行 Python CLI）：
- 导出当前 profile（展平 JSON，仅扩展）：
  `python main.py export current`
- 导出到仓库片段（写入 `extensions/<name>.txt` 等）：
  `python main.py repo-export current --with-settings`
- 从仓库片段安装 profile（先安装 `extensions/common.txt` 再安装 `extensions/<profile>.txt`）：
  `python main.py repo-install python`
- 根据多个 profile 自动抽取 common（并更新 `extensions/common.txt`）：
  `python main.py repo-dedupe`
- 合并 settings/keybindings（备份原有用户配置）：
  `python main.py repo-install python --with-settings --with-keybindings`- 从 repo 片段组装为可导入的 `.code-profile`（可选 VS Code 原生风格单行转义）：
  `python main.py repo-assemble python --with-extensions --with-settings --with-keybindings -o ./Python.code-profile`
  `python tools/assemble_code_profile.py --from-repo python --with-extensions --with-settings --with-keybindings --vscode-style -o ./Python.code-profile`
最佳实践
1. 只把声明性文件（`extensions/*.txt`, `settings/*.json`, `keybindings/*.json`）放进 Git。不要提交 VS Code 的 runtime/state 文件。
2. 新增扩展：先搜索 `extensions/common.txt`，若为项目/角色专用再添加到对应 profile。
3. 把 workspace-specific 插件放到项目的 `.vscode/extensions.json`（recommendedExtensions）。

常见命令
- 安装清单（跨平台）：
  `cat extensions/common.txt | xargs -L1 code --install-extension`  
  （Windows PowerShell 可用：`Get-Content extensions/common.txt | ForEach-Object { code --install-extension $_ }`）

迁移建议流程
1. 运行 `python main.py export current` 生成当前快照（或使用 `--with-settings` 导出设置）。
2. 把导出的扩展 id 拷贝到 `extensions/<profile>.txt`（删除版本号）。
3. 运行 `python main.py repo-dedupe` 抽取公共扩展并清理重复项。
4. 提交并通过 CI / DevContainer 验证仓库中的 profile（可在 CI 中运行 `python main.py repo-install <profile>`）。

需要我：
- 帮你把当前机器导出的 profile 自动拆分到 `extensions/`（我可以在仓库中运行导出并生成初步拆分清单）；
- 或者仅生成更完整的 cross-platform bash 版本脚本（供非 Windows 使用）。

---

## 新：跨平台 Python CLI（展平的 profile JSON） 🐍

说明：仓库现在支持“展平的”profile 格式（示例：`python.json`），而 `Settings.json` 与 `KeyBoards.json` 保持为共享片段。

主要命令示例：
- 列出可用 profile：
  `python main.py list`  
- 从当前 VS Code 导出（仅扩展）：
  `python main.py export current`  → 生成 `./current.json`（仅 extensions）
- 导入 profile（默认只安装扩展）：
  `python main.py import python`  
- 若要同时导入 settings 或 keybindings：
  `python main.py import python --with-settings --with-keybindings`

默认行为：`import` 命令**只安装 extensions**，除非显式加上 `--with-settings` 或 `--with-keybindings`。

在 Windows/macOS/Linux 上均可运行（使用 VS Code 的 `code` CLI 来安装扩展；若系统未安装 `code`，程序会仍能导出/合并本地设置，但无法安装扩展）。

