# nassistant DuiLib Launcher (xmake)

这个目录是新的迁移起点：`C++ + DuiLib + xmake`。

## 目标

- 先搭可运行的 DuiLib 无边框窗口骨架（纯代码生成 UI，不依赖 XML）
- 再逐步迁移启动器能力（分组/项目、启动、拖拽导入、快捷方式解析）

## 目录

- `src/` 应用入口与窗口逻辑
- `xmake.lua` 构建脚本

## 构建

在本目录执行：

```powershell
xmake f -p windows -a x64 -m release
xmake
xmake run nassistant-duilib
```

> 依赖说明：DuiLib 代码现在以 git submodule 形式放在 `third_party/DuiLib_DuiEditor` 下。克隆仓库后请运行：
> 
> ```bash
> git submodule update --init --recursive
> ```
> 
> 构建脚本会引用子模块中的 `DuiLib` 子目录（`../third_party/DuiLib_DuiEditor/DuiLib`）。
> 静态库方式编译链接与之前保持一致。

## 下一步迁移顺序（建议）

1. 接入 `Data.json` 读取与模型映射
2. 渲染分组列表 + 启动项列表
3. 接入点击启动（ShellExecute）
4. 接入拖拽导入与 `.lnk` 目标解析
5. 补编辑窗口与图标缓存
