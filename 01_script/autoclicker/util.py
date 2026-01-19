import win32gui
import win32api
import win32con
import os
import json


def find_window_by_title(title_substring):
    title = title_substring.strip().lower()

    def enum_windows_callback(hwnd, windows):
        if win32gui.IsWindowVisible(hwnd):
            window_title = win32gui.GetWindowText(hwnd)
            if title in window_title.lower():
                windows.append((hwnd, window_title))
        return True

    windows = []
    win32gui.EnumWindows(enum_windows_callback, windows)
    return windows[0] if windows else (None, None)


def screen_to_client(hwnd, x, y):
    # Convert screen (x,y) to client coordinates relative to hwnd
    try:
        # win32gui.ScreenToClient accepts a tuple but returns (x,y)
        cx, cy = win32gui.ScreenToClient(hwnd, (int(x), int(y)))
        return int(cx), int(cy)
    except Exception:
        return None


def post_click_to_window(hwnd, x_screen, y_screen, button='left'):
    """Post a mouse click message to the target window at given screen coordinates.
    x_screen,y_screen are screen coordinates. We convert to client coordinates and post messages.
    """
    if not hwnd:
        return False
    try:
        client = screen_to_client(hwnd, x_screen, y_screen)
        if not client:
            return False
        cx, cy = client
        lParam = (cy << 16) | (cx & 0xffff)

        # Attempt to activate/focus the window before posting click messages.
        # We avoid physically moving the mouse; we either set the foreground window
        # (may fail if OS prevents it) or post activation/focus messages so the
        # target window will process the incoming mouse messages.
        try:
            try:
                # Prefer the Win32 API to bring the window foreground if possible
                win32gui.SetForegroundWindow(hwnd)
            except Exception:
                # Fall back to posting activation messages (no cursor movement)
                win32api.PostMessage(hwnd, win32con.WM_ACTIVATE, win32con.WA_CLICKACTIVE, 0)
                win32api.PostMessage(hwnd, win32con.WM_SETFOCUS, 0, 0)
        except Exception:
            # Ignore failures to focus; we'll still attempt to post the mouse messages
            pass

        if button == 'left':
            win32api.PostMessage(hwnd, win32con.WM_MOUSEMOVE, 0, lParam)
            win32api.PostMessage(hwnd, win32con.WM_LBUTTONDOWN, win32con.MK_LBUTTON, lParam)
            win32api.PostMessage(hwnd, win32con.WM_LBUTTONUP, 0, lParam)
        else:
            win32api.PostMessage(hwnd, win32con.WM_MOUSEMOVE, 0, lParam)
            win32api.PostMessage(hwnd, win32con.WM_RBUTTONDOWN, win32con.MK_RBUTTON, lParam)
            win32api.PostMessage(hwnd, win32con.WM_RBUTTONUP, 0, lParam)
        return True
    except Exception:
        return False


def is_point_in_window(hwnd, x_screen, y_screen):
    try:
        rect = win32gui.GetWindowRect(hwnd)
        left, top, right, bottom = rect
        return left <= x_screen <= right and top <= y_screen <= bottom
    except Exception:
        return False


def config_path(filename: str) -> str:
    """Return the full path to the config file stored in this package directory."""
    base = os.path.dirname(__file__)
    return os.path.join(base, filename)


def load_json_file(filename: str, default=None):
    path = config_path(filename)
    try:
        if not os.path.exists(path):
            return default
        with open(path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception:
        return default


def save_json_file(filename: str, data):
    path = config_path(filename)
    try:
        with open(path, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        return True
    except Exception:
        return False
