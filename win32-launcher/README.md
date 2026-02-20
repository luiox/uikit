# nassistant Win32 版本（Rust + 裸 Win32）

这是无 WebView 的原生 Win32 实现，用于低占用与高响应验证。

## 运行

```bash
cd win32-launcher
cargo run --release
```

## 当前能力

- 读取 `%LOCALAPPDATA%/nassistant/launcher.v2.json`
- 左侧分组列表、右侧启动项列表
- 选中启动项后点击“启动”执行
- 状态栏反馈

## 下一步

- 拖拽导入（含 `.lnk` 解析）
- 编辑启动项
- 图标显示与缓存
- 托盘与全局快捷键
