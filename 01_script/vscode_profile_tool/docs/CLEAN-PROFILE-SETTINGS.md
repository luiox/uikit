# 清理 profile 中多余插件的 settings 🔧

简述
- 本文档说明如何使用 `tools/clean_profile_settings.py` 从已展开的 VS Code profile JSON 中移除那些属于**未列在 profile 的扩展**的设置（例如 `doxdocgen.*`）。

快速开始
```bash
# 展开 .code-profile 为 JSON（你已有的工具）
python tools/expand_code_profile.py mcu_xmake.code-profile.fixed.json -o expanded.json

# 先干运行，查看将被移除的 keys
python tools/clean_profile_settings.py expanded.json --dry-run --report

# 备份并写回（就地替换）
python tools/clean_profile_settings.py expanded.json --backup

# 或写入新文件
python tools/clean_profile_settings.py expanded.json -o expanded.clean.json
```

说明与策略
- 脚本会识别 `extensions` 列表，并把扩展 id（例如 `cschlosser.doxdocgen`）的短名（`doxdocgen`）作为可能的 settings 前缀来保留对应配置。
- 保留所有常见的内置前缀（`editor`, `workbench`, `files`, `python`, ...）及语言作用域设置（`[python]`）。
- 其余以 `prefix.` 开头的设置会被视为来自“多余插件”并被移除。

提示
- 使用 `--keep` 可显式保留自定义前缀，例如 `--keep someOtherExt`。
- 先用 `--dry-run --report` 检查再写入以免误删。