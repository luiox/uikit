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
from .config import AppConfig, BasicConfig, AdvancedConfig
from .hotkeys import HotkeyManager
from .controller import MouseControllerCore
import pyautogui
from . import util
from dataclasses import asdict


def main():
    root = tk.Tk()

    # default safefails
    pyautogui.FAILSAFE = True

    config = AppConfig()
    # Try to load persisted configs if present
    bdata = util.load_json_file('basic.json', None)
    if bdata:
        # convert click_point list to tuple if necessary
        if bdata.get('click_point') and isinstance(bdata.get('click_point'), list):
            bdata['click_point'] = tuple(bdata['click_point'])
        try:
            config.basic = BasicConfig(**bdata)
        except Exception:
            pass
    adata = util.load_json_file('advanced.json', None)
    if adata:
        try:
            config.advanced = AdvancedConfig(**adata)
        except Exception:
            pass
    # Load UI settings
    uiset = util.load_json_file('ui.json', None)
    if uiset and 'geometry' in uiset:
        try:
            root.geometry(uiset['geometry'])
        except Exception:
            pass
    hotkey_manager = HotkeyManager(root)

    controller = MouseControllerCore(on_status=lambda text, fg=None: None,
                                     on_error=lambda msg: messagebox.showerror('错误', msg))

    ui = MainUI(root, config, controller, hotkey_manager)
    # apply loaded configs to pages
    try:
        ui.basic_page.set_from_config(config.basic)
        ui.advanced_page.set_from_config(config.advanced)
    except Exception:
        pass
    # connect controller callbacks to UI now that UI exists
    controller.on_status = lambda text, fg=None: ui.set_status(text, fg)
    controller.on_error = lambda msg: messagebox.showerror('错误', msg)

    root.protocol('WM_DELETE_WINDOW', ui.on_closing)

    root.mainloop()


if __name__ == '__main__':
    main()
