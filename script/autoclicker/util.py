import win32gui
import win32api
import win32con


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
