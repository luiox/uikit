import threading
import time
import pyautogui
import win32gui
from dataclasses import asdict
from typing import Callable


class MouseControllerCore:
    def __init__(self, on_status=None, on_error=None):
        # Callbacks from UI
        self.on_status = on_status or (lambda s, fg=None: None)
        self.on_error = on_error or (lambda msg: None)

        self._stop_event = threading.Event()
        self._thread = None
        self._mode = None
        self._config_snapshot = None

    def start(self, mode: str, config_snapshot, mouseio=None):
        if self.is_running():
            return False
        self._stop_event.clear()
        self._mode = mode
        self._config_snapshot = config_snapshot
        self._mouseio = mouseio or pyautogui

        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()
        return True

    def stop(self):
        self._stop_event.set()
        if self._thread and self._thread.is_alive():
            self._thread.join(timeout=1)
        self._thread = None

    def is_running(self):
        return self._thread is not None and self._thread.is_alive()

    def _run(self):
        try:
            if self._mode == 'basic':
                self._run_basic()
            elif self._mode == 'advanced':
                self._run_advanced()
        except Exception as e:
            self.on_error(str(e))
        finally:
            self.on_status('Stopped', 'green')

    def _run_basic(self):
        cfg = self._config_snapshot.basic

        if not cfg.click_point:
            self.on_error('Basic mode requires a click point (通过坐标输入/Overlay/F9 捕获设置)')
            return

        x, y = cfg.click_point
        interval = float(cfg.interval)
        button = cfg.click_button

        self.on_status('Running (Basic)', 'red')

        # If restrict_to_window is enabled, try to find window and send messages directly
        if cfg.restrict_to_window and cfg.target_window_title:
            # find window
            hwnd, _ = None, None
            windows = []
            def enum_windows(hwnd2, lst):
                import win32gui
                if win32gui.IsWindowVisible(hwnd2):
                    wt = win32gui.GetWindowText(hwnd2)
                    if cfg.target_window_title.lower() in wt.lower():
                        lst.append((hwnd2, wt))
                return True
            import win32gui
            win32gui.EnumWindows(enum_windows, windows)
            if windows:
                hwnd, _ = windows[0]
            else:
                self.on_error('未找到目标窗口，请先查找或输入正确的标题')
                return

            # If enabled, check if the mouse is inside the window and optionally stop when leaving
            while not self._stop_event.is_set():
                from . import util as _util
                # check stop on leave
                if cfg.stop_on_leave:
                    mx, my = self._mouseio.position()
                    if not _util.is_point_in_window(hwnd, mx, my):
                        self.on_status('离开窗口, 停止', 'green')
                        break
                # post click to window
                ok = _util.post_click_to_window(hwnd, x, y, button=button)
                if not ok:
                    self.on_error('无法向目标窗口发送点击消息')
                    break
                time.sleep(interval)
            return

        # Default: physically move the mouse and click at the screen position
        self._mouseio.moveTo(x, y)
        time.sleep(0.2)

        while not self._stop_event.is_set():
            if button == 'left':
                self._mouseio.click(button='left')
            else:
                self._mouseio.click(button='right')
            time.sleep(interval)

    def _run_advanced(self):
        cfg = self._config_snapshot.advanced
        # Set FAILSAFE according to config
        pyautogui.FAILSAFE = cfg.fail_safe

        # Find window, set foreground
        hwnd, title = (None, '')
        if cfg.target_window_title:
            windows = []
            def enum_windows(hwnd2, lst):
                if win32gui.IsWindowVisible(hwnd2):
                    wt = win32gui.GetWindowText(hwnd2)
                    if cfg.target_window_title.lower() in wt.lower():
                        lst.append((hwnd2, wt))
                return True
            win32gui.EnumWindows(enum_windows, windows)
            if windows:
                hwnd, title = windows[0]
                try:
                    win32gui.SetForegroundWindow(hwnd)
                except Exception:
                    pass

        self.on_status('Running (Advanced)', 'red')

        # try to center
        if hwnd:
            rect = win32gui.GetWindowRect(hwnd)
            center_x = (rect[0] + rect[2]) // 2
            center_y = (rect[1] + rect[3]) // 2
            self._mouseio.moveTo(center_x, center_y)
            time.sleep(0.3)

        click_count = 0
        while not self._stop_event.is_set():
            # check for failsafe spot (corner)
            x, y = self._mouseio.position()
            screen_w, screen_h = self._mouseio.size()
            if x < 10 or y < 10 or x > screen_w - 10 or y > screen_h - 10:
                self.on_status('Emergency stop', 'green')
                break

            # movement
            move_distance = cfg.move_range
            move_speed = cfg.move_speed

            # Right move
            steps = max(1, int(move_distance / max(1, move_speed)))
            for _ in range(steps):
                if self._stop_event.is_set():
                    break
                self._mouseio.moveRel(move_speed, 0)
                time.sleep(0.01)

            # Left move
            for _ in range(steps):
                if self._stop_event.is_set():
                    break
                self._mouseio.moveRel(-move_speed, 0)
                time.sleep(0.01)

            # Alternate click
            if click_count % 2 == 0:
                self._mouseio.click(button='left')
            else:
                self._mouseio.click(button='right')
            click_count += 1

            time.sleep(cfg.click_interval)
