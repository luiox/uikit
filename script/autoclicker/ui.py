import tkinter as tk
from tkinter import ttk, messagebox
from .pages.basic_page import BasicPage
from .pages.advanced_page import AdvancedPage
from .config import AppConfig
from . import util
import pyautogui

class MainUI:
    def __init__(self, root, config: AppConfig, controller, hotkey_manager):
        self.root = root
        self.config = config
        self.controller = controller
        self.hotkey_manager = hotkey_manager

        self.root.title('鼠标控制器')
        self.root.geometry('600x600')

        self.notebook = ttk.Notebook(self.root)
        self.basic_page = BasicPage(self.notebook, hotkey_manager=self.hotkey_manager)
        self.advanced_page = AdvancedPage(self.notebook)
        self.notebook.add(self.basic_page, text='基础')
        self.notebook.add(self.advanced_page, text='高级')
        self.notebook.pack(fill='both', expand=True)

        # Central control
        control_frame = ttk.Frame(self.root, padding=10)
        control_frame.pack(anchor='center')
        self.start_button = ttk.Button(control_frame, text='手动开始', command=self.start_control)
        self.start_button.grid(row=0, column=0, padx=5)
        self.stop_button = ttk.Button(control_frame, text='手动停止', command=self.stop_control, state='disabled')
        self.stop_button.grid(row=0, column=1, padx=5)

        # Status label
        self.status_var = tk.StringVar(value='待机中')
        self.status_label = ttk.Label(self.root, textvariable=self.status_var, font=('Arial', 12))
        self.status_label.pack(pady=5)
        # Hotkey status
        self.hotkey_var = tk.StringVar(value='热键: 未注册')
        self.hotkey_label = ttk.Label(self.root, textvariable=self.hotkey_var, font=('Arial', 10))
        self.hotkey_label.pack()

        # Bind tab change to register/unregister F9 accordingly
        self.notebook.bind('<<NotebookTabChanged>>', self._on_tab_change)

        # Register global F8 hotkey
        try:
            self.hotkey_manager.register_f8(self._on_global_f8)
        except Exception as e:
            # fallback: hotkey_manager may auto fallback to keyboard module
            pass

        # Ensure initial tab activation is handled
        self._on_tab_change(None)
        self._update_hotkey_labels()

    def _on_tab_change(self, event):
        current = self.notebook.select()
        # Determine index
        idx = self.notebook.index(current)
        if idx == 0:  # Basic
            # register F9 only for basic
            try:
                self.hotkey_manager.register_f9(self._on_global_f9)
            except Exception:
                pass
        else:
            try:
                self.hotkey_manager.unregister_f9()
            except Exception:
                pass
        self._update_hotkey_labels()

    def _update_hotkey_labels(self):
        f8_reg = self.hotkey_manager.is_registered('F8')
        f9_reg = self.hotkey_manager.is_registered('F9')
        kb_status = 'F8: ' + ('已注册' if f8_reg else '未注册') + ' | F9: ' + ('已注册' if f9_reg else '未注册')
        self.hotkey_var.set(kb_status)

    def _on_global_f8(self):
        # Toggle start/stop
        if self.controller.is_running():
            self.stop_control()
        else:
            self.start_control()

    def _on_global_f9(self):
        # Only effective in Basic page
        idx = self.notebook.index(self.notebook.select())
        if idx != 0:
            return
        # capture current mouse position
        x, y = pyautogui.position()
        self.basic_page._on_capture((x, y))

    def start_control(self):
        # Prepare config snapshot
        mode = 'basic' if self.notebook.index(self.notebook.select()) == 0 else 'advanced'
        # Update config from pages
        self.config.basic = self.basic_page.get_config()
        self.config.advanced = self.advanced_page.get_config()

        # If target mode is 'cursor', sample the current mouse position at start
        if mode == 'basic' and self.config.basic.target_mode == 'cursor':
            try:
                x, y = pyautogui.position()
                self.config.basic.click_point = (int(x), int(y))
            except Exception:
                pass

        if mode == 'basic' and not self.config.basic.click_point:
            messagebox.showwarning('警告', '请先设置点击坐标（输入/Overlay/F9）')
            return

        # if basic restrict to window is enabled then ensure the target window exists
        if mode == 'basic' and self.config.basic.restrict_to_window:
            title = self.config.basic.target_window_title.strip()
            if not title:
                messagebox.showwarning('警告', '已启用 "限定特定窗口"，请先填写目标窗口标题并点击查找')
                return
            hwnd, wt = util.find_window_by_title(title)
            if not hwnd:
                messagebox.showwarning('警告', f'未找到包含 "{title}" 的窗口，请检查标题或先使用查找按钮')
                return

        # apply FAILSAFE option
        try:
            pyautogui.FAILSAFE = self.config.advanced.fail_safe
        except Exception:
            pass

        started = self.controller.start(mode, self.config)
        if started:
            self.start_button.config(state='disabled')
            self.stop_button.config(state='normal')
            self.status_var.set('运行中...')

    def stop_control(self):
        self.controller.stop()
        self.start_button.config(state='normal')
        self.stop_button.config(state='disabled')
        self.status_var.set('待机中')

    def set_status(self, text, color=None):
        self.status_var.set(text)
        if color:
            try:
                self.status_label.config(foreground=color)
            except Exception:
                pass
        # If the controller stopped or there was an emergency, reset start/stop buttons
        low = (text or '').lower()
        if 'stopped' in low or '停止' in low or '离开' in low or '待机' in low:
            try:
                self.start_button.config(state='normal')
                self.stop_button.config(state='disabled')
            except Exception:
                pass

    def on_closing(self):
        # Cleanup
        try:
            self.hotkey_manager.close()
        except Exception:
            pass
        self.controller.stop()
        self.root.destroy()
