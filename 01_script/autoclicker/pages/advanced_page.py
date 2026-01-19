import tkinter as tk
from tkinter import ttk, messagebox
from ..config import AdvancedConfig
from .. import util

class AdvancedPage(ttk.Frame):
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)

        self.move_speed_var = tk.DoubleVar(value=5.0)
        self.move_range_var = tk.IntVar(value=100)
        self.interval_var = tk.DoubleVar(value=0.5)
        self.window_title_var = tk.StringVar(value='')
        self.fail_safe_var = tk.BooleanVar(value=True)

        self.setup_ui()

    def setup_ui(self):
        frame = ttk.Frame(self, padding=10)
        frame.grid(row=0, column=0, sticky='nsew')

        ttk.Label(frame, text='移动速度:').grid(row=0, column=0, sticky='w')
        ttk.Spinbox(frame, from_=1, to=50, increment=1, textvariable=self.move_speed_var, width=6).grid(row=0, column=1)

        ttk.Label(frame, text='移动范围:').grid(row=1, column=0, sticky='w')
        ttk.Spinbox(frame, from_=10, to=1000, increment=10, textvariable=self.move_range_var, width=6).grid(row=1, column=1)

        ttk.Label(frame, text='点击间隔(秒):').grid(row=2, column=0, sticky='w')
        ttk.Spinbox(frame, from_=0.05, to=10, increment=0.05, textvariable=self.interval_var, width=6).grid(row=2, column=1)

        ttk.Label(frame, text='目标窗口标题:').grid(row=3, column=0, sticky='w')
        ttk.Entry(frame, textvariable=self.window_title_var, width=30).grid(row=3, column=1, columnspan=2)
        ttk.Button(frame, text='查找窗口', command=self.find_window).grid(row=3, column=3)

        ttk.Checkbutton(frame, text='关闭 FAILSAFE（危险）', variable=self.fail_safe_var).grid(row=4, column=0, columnspan=2)

    def find_window(self):
        title = self.window_title_var.get().strip()
        if not title:
            messagebox.showwarning('警告', '请输入窗口标题')
            return
        hwnd, wt = util.find_window_by_title(title)
        if hwnd:
            messagebox.showinfo('成功', f'找到窗口: {wt}')
        else:
            messagebox.showerror('错误', f'未找到包含 "{title}" 的窗口')

    def get_config(self) -> AdvancedConfig:
        return AdvancedConfig(
            move_speed=self.move_speed_var.get(),
            move_range=self.move_range_var.get(),
            click_interval=self.interval_var.get(),
            target_window_title=self.window_title_var.get(),
            fail_safe=self.fail_safe_var.get(),
        )

    def set_from_config(self, cfg: AdvancedConfig):
        self.move_speed_var.set(cfg.move_speed)
        self.move_range_var.set(cfg.move_range)
        self.interval_var.set(cfg.click_interval)
        self.window_title_var.set(cfg.target_window_title)
        self.fail_safe_var.set(cfg.fail_safe)

    def on_activate(self):
        pass

    def on_deactivate(self):
        pass
