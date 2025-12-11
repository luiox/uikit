import tkinter as tk
from tkinter import messagebox

# app.py 顶部（放在任何相对 import 之前）
if __package__ is None and __name__ == "__main__":
    import sys, os
    pkg_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(pkg_dir)
    if parent_dir not in sys.path:
        sys.path.insert(0, parent_dir)
    __package__ = "autoclicker"

from .ui import MainUI
from .config import AppConfig
from .hotkeys import HotkeyManager
from .controller import MouseControllerCore
import pyautogui


def main():
    root = tk.Tk()

    # default safefails
    pyautogui.FAILSAFE = True

    config = AppConfig()
    hotkey_manager = HotkeyManager(root)

    controller = MouseControllerCore(on_status=lambda text, fg=None: None,
                                     on_error=lambda msg: messagebox.showerror('错误', msg))

    ui = MainUI(root, config, controller, hotkey_manager)
    # connect controller callbacks to UI now that UI exists
    controller.on_status = lambda text, fg=None: ui.set_status(text, fg)
    controller.on_error = lambda msg: messagebox.showerror('错误', msg)

    root.protocol('WM_DELETE_WINDOW', ui.on_closing)

    root.mainloop()


if __name__ == '__main__':
    main()
