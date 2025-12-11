# Autoclicker (refactor)

This package refactors the previous `auto_clicker.py` single-file app into a modular package with a Basic and Advanced UI pages.

- Basic: single left/right click, choose interval, set click point via overlay/F9/manual input
	- 新增安全选项：可勾选“限定特定窗口”，启用后使用窗口消息发送点击（不会移动全局鼠标）
	- 勾选“离开窗口停止连点”以在鼠标离开目标窗口时自动停止点击
- Advanced: original behavior (find window, move left/right, alternate clicks), with FAILSAFE setting

Run with:

```powershell
python -m autoclicker
```

Dependencies: `pyautogui`, `keyboard`, `pywin32` (Windows only)
