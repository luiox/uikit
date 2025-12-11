# Autoclicker (refactor)

This package refactors the previous `auto_clicker.py` single-file app into a modular package with a Basic and Advanced UI pages.

- Basic: single left/right click, choose interval, set click point via overlay/F9/manual input
- Advanced: original behavior (find window, move left/right, alternate clicks), with FAILSAFE setting

Run with:

```powershell
python -m autoclicker
```

Dependencies: `pyautogui`, `keyboard`, `pywin32` (Windows only)
