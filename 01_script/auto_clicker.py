import tkinter as tk
from tkinter import ttk, messagebox
import threading
import time
import win32gui
import win32con
import win32api
import keyboard
import pyautogui
import sys
from ctypes import windll, wintypes, byref


class MouseController:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("鼠标控制器 - F8触发")
        self.root.geometry("400x500")
        self.root.resizable(True, True)

        # 控制变量
        self.is_running = False
        self.control_thread = None
        self.target_window = None
        self.window_title = ""
        # 全局热键变量
        self.hotkey_registered = False
        self.hotkey_listener_thread = None
        self.listener_thread_id = None

        # 参数变量
        self.move_speed = tk.DoubleVar(value=5.0)  # 移动速度（像素/次）
        self.click_interval = tk.DoubleVar(value=0.5)  # 点击间隔（秒）
        self.move_range = tk.IntVar(value=100)  # 移动范围（像素）
        self.target_window_title = tk.StringVar(value="目标窗口标题")

        # 设置UI
        self.setup_ui()

        # 绑定F8热键
        # Prefer a proper system-wide hotkey (via RegisterHotKey); fallback to keyboard module
        try:
            self.register_hotkey()
        except Exception:
            # If registration fails, fall back to keyboard module
            keyboard.on_press_key("F8", self.on_f8_press)

        # 禁用pyautogui的安全机制
        pyautogui.FAILSAFE = False

        # 关闭窗口时的处理
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)

    def setup_ui(self):
        # 主框架
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        # 标题
        title_label = ttk.Label(main_frame, text="鼠标控制器", font=("Arial", 16, "bold"))
        title_label.grid(row=0, column=0, columnspan=2, pady=10)

        # 说明
        info_label = ttk.Label(main_frame, text="按F8键开始/停止控制", font=("Arial", 10))
        info_label.grid(row=1, column=0, columnspan=2, pady=5)

        # 分隔线
        ttk.Separator(main_frame, orient='horizontal').grid(row=2, column=0, columnspan=2, sticky='ew', pady=10)

        # 目标窗口设置
        window_frame = ttk.LabelFrame(main_frame, text="目标窗口设置", padding="10")
        window_frame.grid(row=3, column=0, columnspan=2, sticky='ew', pady=5)

        ttk.Label(window_frame, text="窗口标题:").grid(row=0, column=0, sticky=tk.W)
        window_entry = ttk.Entry(window_frame, textvariable=self.target_window_title, width=30)
        window_entry.grid(row=0, column=1, padx=5)

        ttk.Button(window_frame, text="查找窗口", command=self.find_window).grid(row=0, column=2, padx=5)

        self.window_status = ttk.Label(window_frame, text="未找到窗口", foreground="red")
        self.window_status.grid(row=1, column=0, columnspan=3, pady=5)

        # 移动参数设置
        move_frame = ttk.LabelFrame(main_frame, text="移动参数", padding="10")
        move_frame.grid(row=4, column=0, columnspan=2, sticky='ew', pady=5)

        # 移动速度
        ttk.Label(move_frame, text="移动速度:").grid(row=0, column=0, sticky=tk.W)
        speed_scale = ttk.Scale(move_frame, from_=1, to=20, variable=self.move_speed,
                                orient=tk.HORIZONTAL, length=200)
        speed_scale.grid(row=0, column=1, padx=5)
        self.speed_label = ttk.Label(move_frame, text="5.0 px")
        self.speed_label.grid(row=0, column=2)
        speed_scale.config(command=lambda v: self.speed_label.config(text=f"{float(v):.1f} px"))

        # 移动范围
        ttk.Label(move_frame, text="移动范围:").grid(row=1, column=0, sticky=tk.W, pady=5)
        range_scale = ttk.Scale(move_frame, from_=50, to=300, variable=self.move_range,
                                orient=tk.HORIZONTAL, length=200)
        range_scale.grid(row=1, column=1, padx=5, pady=5)
        self.range_label = ttk.Label(move_frame, text="100 px")
        self.range_label.grid(row=1, column=2, pady=5)
        range_scale.config(command=lambda v: self.range_label.config(text=f"{int(float(v))} px"))

        # 点击参数设置
        click_frame = ttk.LabelFrame(main_frame, text="点击参数", padding="10")
        click_frame.grid(row=5, column=0, columnspan=2, sticky='ew', pady=5)

        # 点击间隔
        ttk.Label(click_frame, text="点击间隔:").grid(row=0, column=0, sticky=tk.W)
        interval_scale = ttk.Scale(click_frame, from_=0.1, to=2.0, variable=self.click_interval,
                                   orient=tk.HORIZONTAL, length=200)
        interval_scale.grid(row=0, column=1, padx=5)
        self.interval_label = ttk.Label(click_frame, text="0.5 秒")
        self.interval_label.grid(row=0, column=2)
        interval_scale.config(command=lambda v: self.interval_label.config(text=f"{float(v):.1f} 秒"))

        # 状态显示
        status_frame = ttk.LabelFrame(main_frame, text="状态", padding="10")
        status_frame.grid(row=6, column=0, columnspan=2, sticky='ew', pady=5)

        self.status_label = ttk.Label(status_frame, text="待机中", font=("Arial", 12), foreground="green")
        self.status_label.grid(row=0, column=0)
        # 全局热键状态
        self.hotkey_label = ttk.Label(status_frame, text="热键: 未注册", font=("Arial", 10), foreground="orange")
        self.hotkey_label.grid(row=0, column=1, padx=10)

        # 控制按钮
        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=7, column=0, columnspan=2, pady=10)

        self.start_button = ttk.Button(button_frame, text="手动开始", command=self.start_control)
        self.start_button.grid(row=0, column=0, padx=5)

        self.stop_button = ttk.Button(button_frame, text="手动停止", command=self.stop_control, state=tk.DISABLED)
        self.stop_button.grid(row=0, column=1, padx=5)

        # 使用说明
        help_frame = ttk.LabelFrame(main_frame, text="使用说明", padding="10")
        help_frame.grid(row=8, column=0, columnspan=2, sticky='ew', pady=5)

        help_text = """1. 输入目标窗口标题，点击"查找窗口"
2. 调整移动和点击参数
3. 按F8键或点击"手动开始"开始控制
4. 再次按F8键或点击"手动停止"停止控制
5. 移动鼠标到屏幕角落可紧急停止"""
        # Clarify that F8 tries to register as a global hotkey and may fail under extreme exclusive-input mode
        help_text += "\n\n注意: 程序会尝试注册全局热键 F8，通常可在游戏中工作；若注册失败，会回退为普通本地按键绑定。"

        help_label = ttk.Label(help_frame, text=help_text, justify=tk.LEFT)
        help_label.grid(row=0, column=0)

    def find_window(self):
        """查找目标窗口"""
        title = self.target_window_title.get().strip()
        if not title:
            messagebox.showwarning("警告", "请输入窗口标题")
            return

        def enum_windows_callback(hwnd, windows):
            if win32gui.IsWindowVisible(hwnd):
                window_title = win32gui.GetWindowText(hwnd)
                if title.lower() in window_title.lower():
                    windows.append((hwnd, window_title))
            return True

        windows = []
        win32gui.EnumWindows(enum_windows_callback, windows)

        if windows:
            self.target_window = windows[0][0]
            self.window_title = windows[0][1]
            self.window_status.config(text=f"找到: {self.window_title}", foreground="green")
            messagebox.showinfo("成功", f"找到窗口: {self.window_title}")
        else:
            self.target_window = None
            self.window_title = ""
            self.window_status.config(text="未找到窗口", foreground="red")
            messagebox.showerror("错误", f"未找到包含 '{title}' 的窗口")

    def on_f8_press(self, event):
        """F8键按下事件"""
        # This is used as a fallback local keyboard binding
        if event.name == 'F8':
            if self.is_running:
                self.stop_control()
            else:
                self.start_control()

    def register_hotkey(self):
        """尝试注册全局F8热键并启动消息循环线程"""
        MOD_NONE = 0x0000
        VK_F8 = 0x77
        WM_HOTKEY = 0x0312

        user32 = windll.user32

        # Try to register hotkey globally on the thread message queue
        if not user32.RegisterHotKey(None, 1, MOD_NONE, VK_F8):
            # Registration failed; fallback to keyboard local binding
            self.hotkey_label.config(text="热键: 未注册", foreground="red")
            messagebox.showwarning("警告", "无法注册全局热键 F8，可能已被其他程序占用。将使用本地按键作为回退。")
            keyboard.on_press_key("F8", self.on_f8_press)
            return

        # Registered successfully
        self.hotkey_registered = True
        self.hotkey_label.config(text="热键: F8（全局）", foreground="green")
        self.hotkey_listener_thread = threading.Thread(target=self.hotkey_listener, daemon=True)
        self.hotkey_listener_thread.start()

    def hotkey_listener(self):
        """消息循环线程，监听 WM_HOTKEY 事件并触发处理（在主线程通过 root.after 调用）"""
        user32 = windll.user32
        kernel32 = windll.kernel32
        WM_HOTKEY = 0x0312

        # Store the thread id so we can post WM_QUIT to it later
        try:
            self.listener_thread_id = kernel32.GetCurrentThreadId()
        except Exception:
            self.listener_thread_id = None

        msg = wintypes.MSG()
        while True:
            res = user32.GetMessageW(byref(msg), None, 0, 0)
            # res == 0 means WM_QUIT; res == -1 is error
            if res == 0:
                break
            if res == -1:
                break
            if msg.message == WM_HOTKEY and msg.wParam == 1:
                # Schedule the on_global_f8 to run in the Tk main thread
                try:
                    self.root.after(0, self.on_global_f8)
                except Exception:
                    pass
            user32.TranslateMessage(byref(msg))
            user32.DispatchMessageW(byref(msg))

    def on_global_f8(self):
        """Handler called by the RegisterHotKey message loop (runs in main thread)."""
        if self.is_running:
            self.stop_control()
        else:
            self.start_control()

    def start_control(self):
        """开始控制"""
        if not self.target_window:
            messagebox.showwarning("警告", "请先查找目标窗口")
            return

        if self.is_running:
            return

        self.is_running = True
        self.status_label.config(text="运行中...", foreground="red")
        self.start_button.config(state=tk.DISABLED)
        self.stop_button.config(state=tk.NORMAL)

        # 启动控制线程
        self.control_thread = threading.Thread(target=self.control_loop, daemon=True)
        self.control_thread.start()

    def stop_control(self):
        """停止控制"""
        self.is_running = False
        self.status_label.config(text="待机中", foreground="green")
        self.start_button.config(state=tk.NORMAL)
        self.stop_button.config(state=tk.DISABLED)

    def control_loop(self):
        """控制循环"""
        try:
            # 激活目标窗口
            win32gui.SetForegroundWindow(self.target_window)
            time.sleep(0.5)

            # 获取窗口位置
            rect = win32gui.GetWindowRect(self.target_window)
            center_x = (rect[0] + rect[2]) // 2
            center_y = (rect[1] + rect[3]) // 2

            # 移动鼠标到窗口中心
            pyautogui.moveTo(center_x, center_y)
            time.sleep(0.5)

            click_count = 0

            while self.is_running:
                # 检查鼠标是否在屏幕角落（紧急停止）
                x, y = pyautogui.position()
                if x < 10 or y < 10 or x > pyautogui.size().width - 10 or y > pyautogui.size().height - 10:
                    self.root.after(0, self.stop_control)
                    break

                # 左右移动
                move_distance = self.move_range.get()
                move_speed = self.move_speed.get()

                # 向右移动
                for i in range(int(move_distance / move_speed)):
                    if not self.is_running:
                        break
                    pyautogui.moveRel(move_speed, 0)
                    time.sleep(0.01)

                # 向左移动
                for i in range(int(move_distance / move_speed)):
                    if not self.is_running:
                        break
                    pyautogui.moveRel(-move_speed, 0)
                    time.sleep(0.01)

                # 交替点击
                if click_count % 2 == 0:
                    pyautogui.leftClick()
                else:
                    pyautogui.rightClick()
                click_count += 1

                # 等待间隔
                time.sleep(self.click_interval.get())

        except Exception as e:
            self.root.after(0, lambda: messagebox.showerror("错误", f"控制过程中出错: {str(e)}"))
            self.root.after(0, self.stop_control)

    def on_closing(self):
        """窗口关闭时的处理"""
        self.is_running = False
        if self.control_thread and self.control_thread.is_alive():
            self.control_thread.join(timeout=1)
        # 注销全局热键并关闭监听线程
        try:
            if self.hotkey_registered:
                windll.user32.UnregisterHotKey(None, 1)
                # 请求热键监听线程退出 gracefully
                if self.listener_thread_id:
                    windll.user32.PostThreadMessageW(self.listener_thread_id, 0x0012, 0, 0)  # WM_QUIT
                if self.hotkey_listener_thread and self.hotkey_listener_thread.is_alive():
                    self.hotkey_listener_thread.join(timeout=1)
        except Exception:
            pass
        self.root.destroy()

    def run(self):
        """运行应用"""
        self.root.mainloop()


if __name__ == "__main__":
    # New entry: delegate to refactored package if available
    try:
        from autoclicker.app import main as autoclicker_main
        autoclicker_main()
    except Exception:
        # Fallback to original behavior
        try:
            is_admin = windll.shell32.IsUserAnAdmin()
            if not is_admin:
                messagebox.showwarning("警告", "建议以管理员权限运行以确保窗口控制功能正常")
        except Exception:
            pass

        app = MouseController()
        app.run()
