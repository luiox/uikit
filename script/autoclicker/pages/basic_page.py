import tkinter as tk
from tkinter import ttk, messagebox
from ..capture_overlay import CaptureOverlay
from ..config import BasicConfig
from .. import util

class BasicPage(ttk.Frame):
    def __init__(self, parent, hotkey_manager=None, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.hotkey_manager = hotkey_manager

        self.click_button_var = tk.StringVar(value='left')
        self.interval_var = tk.DoubleVar(value=0.5)
        self.coord_x_var = tk.StringVar(value='')
        self.coord_y_var = tk.StringVar(value='')
        self.restrict_var = tk.BooleanVar(value=False)
        self.target_window_title_var = tk.StringVar(value='')
        self.stop_on_leave_var = tk.BooleanVar(value=False)

        self.setup_ui()

    def setup_ui(self):
        frame = ttk.Frame(self, padding=10)
        frame.grid(row=0, column=0, sticky='nsew')

        ttk.Label(frame, text='点击类型:').grid(row=0, column=0, sticky='w')
        ttk.Radiobutton(frame, text='左键', variable=self.click_button_var, value='left').grid(row=0, column=1)
        ttk.Radiobutton(frame, text='右键', variable=self.click_button_var, value='right').grid(row=0, column=2)

        ttk.Label(frame, text='点击间隔(秒):').grid(row=1, column=0, sticky='w')
        ttk.Spinbox(frame, from_=0.05, to=10, increment=0.05, textvariable=self.interval_var, width=6).grid(row=1, column=1)

        ttk.Label(frame, text='坐标 (X, Y):').grid(row=2, column=0, sticky='w')
        ttk.Entry(frame, textvariable=self.coord_x_var, width=8).grid(row=2, column=1)
        ttk.Entry(frame, textvariable=self.coord_y_var, width=8).grid(row=2, column=2)

        ttk.Button(frame, text='Capture (Overlay)', command=self.open_overlay).grid(row=3, column=0, pady=5)
        ttk.Label(frame, text='或按 F9 捕获当前鼠标坐标（仅在 Basic 页面生效）').grid(row=3, column=1, columnspan=2)
        self.last_capture_var = tk.StringVar(value='未捕获')
        ttk.Label(frame, textvariable=self.last_capture_var).grid(row=4, column=0, columnspan=3, sticky='w')

        # 限定窗口选项
        ttk.Label(frame, text='限定窗口标题:').grid(row=5, column=0, sticky='w')
        ttk.Entry(frame, textvariable=self.target_window_title_var, width=30).grid(row=5, column=1, columnspan=2)
        ttk.Button(frame, text='查找窗口', command=self.find_window).grid(row=5, column=3)
        ttk.Checkbutton(frame, text='限定特定窗口（使用窗口消息发送点击，较安全）', variable=self.restrict_var).grid(row=6, column=0, columnspan=3, sticky='w')
        ttk.Checkbutton(frame, text='离开窗口停止连点（仅在限定窗口时生效）', variable=self.stop_on_leave_var).grid(row=7, column=0, columnspan=3, sticky='w')

    def open_overlay(self):
        # Top-level is the root window
        root = self.winfo_toplevel()
        CaptureOverlay(root, self._on_capture)

    def _on_capture(self, point):
        if not point:
            return
        x, y = point
        self.coord_x_var.set(str(x))
        self.coord_y_var.set(str(y))
        self.last_capture_var.set(f'捕获坐标: ({x}, {y})')
        try:
            messagebox.showinfo('捕获', f'捕获坐标: ({x}, {y})')
        except Exception:
            pass

    def get_config(self) -> BasicConfig:
        # Validate inputs
        x = self.coord_x_var.get().strip()
        y = self.coord_y_var.get().strip()
        point = None
        if x and y:
            try:
                px = int(x)
                py = int(y)
                point = (px, py)
            except Exception:
                # ignore invalid
                pass
        return BasicConfig(
            click_button=self.click_button_var.get(),
            interval=float(self.interval_var.get()),
            click_point=point,
            restrict_to_window=self.restrict_var.get(),
            target_window_title=self.target_window_title_var.get(),
            stop_on_leave=self.stop_on_leave_var.get(),
        )

    def set_from_config(self, cfg: BasicConfig):
        self.click_button_var.set(cfg.click_button)
        self.interval_var.set(cfg.interval)
        if cfg.click_point:
            self.coord_x_var.set(str(cfg.click_point[0]))
            self.coord_y_var.set(str(cfg.click_point[1]))
        else:
            self.coord_x_var.set('')
            self.coord_y_var.set('')
        self.restrict_var.set(cfg.restrict_to_window)
        self.target_window_title_var.set(cfg.target_window_title)
        self.stop_on_leave_var.set(cfg.stop_on_leave)

    def find_window(self):
        title = self.target_window_title_var.get().strip()
        if not title:
            messagebox.showwarning('警告', '请输入窗口标题')
            return
        hwnd, wt = util.find_window_by_title(title)
        if hwnd:
            messagebox.showinfo('成功', f'找到窗口: {wt} (hwnd={hwnd})')
        else:
            messagebox.showerror('错误', f'未找到包含 "{title}" 的窗口')

    def on_activate(self):
        # Called when the page becomes active
        pass

    def on_deactivate(self):
        # For unregistering if needed
        pass
