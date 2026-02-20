# icon-lib（独立可搬迁）

这是一个独立的 icon 工程，可单独搬走使用。项目目标是维护“精简、可控”的常用 SVG 集合，并自动生成可直接被 C++ 引用的常量 API（不写死路径字符串）。

## 设计原则
- 只保留业务必需 icon（按 `catalog/icons-100.json`）。
- 所有 SVG 扁平存放在 `icons/`（不区分 feather/remix 来源目录）。
- 通过代码生成产出 `generated/icons.h/.cpp`。
- 外部工程可选择 `dynamic` 或 `embed` 两种 target。

## 目录结构
- `icons/`：最终使用的 SVG 目录（扁平结构，文件名为 `<icon_id>.svg`）。
- `catalog/icons-100.json`：标准 100-icon 清单。
- `generated/icons.h`：生成的枚举与 API 声明（`enum class Icon`）。
- `generated/icons.cpp`：生成的动态/嵌入访问实现。
- `xmake.lua`：独立构建入口（提供 `iconlib_dynamic` 与 `iconlib_embed`）。

## 常用脚本
- `python scripts/generate_cpp_assets.py`
   - 根据 `icons/` + `catalog/icons-100.json` 生成 `icons.h/icons.cpp`。
- `python scripts/check_coverage.py`
   - 检查当前 `icons/` 覆盖率并生成缺口清单。
- `python scripts/build_minimal_feather_pack.py`
   - 从临时 Feather 源提取精简子集到 `icons/`（仅在 bootstrap 阶段使用）。

## 构建（xmake）
- 手工触发生成（仅在 icon 变更后需要）：
   - `xmake iconlib_codegen`
- 然后构建：
   - `xmake f -m release`
   - `xmake build iconlib_dynamic`
   - `xmake build iconlib_embed`

说明：默认构建不会自动重新生成 `generated/icons.h/.cpp`，避免每次 build 失去缓存收益。

## 对外接入方式
在你的主工程里把 `icon-lib` 当子项目引入（例如 `add_subdirs("path/to/icon-lib")`），然后：
- 动态模式：`add_deps("iconlib_dynamic")`
- 嵌入模式：`add_deps("iconlib_embed")`
- 头文件：`#include "icons.h"`

使用示例：
- `iconlib::GetDynamicPath(iconlib::Icon::Search)`
- `iconlib::GetEmbeddedSvg(iconlib::Icon::Search)`
- `iconlib::FindIcon("search")`

## Bootstrap（可选，不属于日常流程）
如果你要从完整 Feather zip 重新提取，可手动执行：
- `./scripts/extract_feather.ps1 -ZipPath "<your-feather-zip-path>" -TargetDir "vendor/feather"`
- 然后执行 `python scripts/build_minimal_feather_pack.py`

日常维护不需要 `vendor/feather`，也不需要根目录 `feather.zip`。
