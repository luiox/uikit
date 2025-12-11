import tkinter as tk

class CaptureOverlay(tk.Toplevel):
    """A full-screen transparent overlay to capture a mouse release coordinate."""

    def __init__(self, root, on_capture):
        super().__init__(root)
        self.root = root
        self.on_capture = on_capture
        self.attributes('-fullscreen', True)
        # Make overlay click-through? No, we need to capture clicks.
        self.attributes('-topmost', True)
        try:
            self.attributes('-alpha', 0.25)
        except Exception:
            pass
        self.configure(bg='black')
        self.bind('<ButtonRelease-1>', self._on_release)
        self.bind('<Escape>', self._on_escape)
        self._label = tk.Label(self, text='拖放并释放以捕获坐标，或按 Esc 取消', fg='white', bg='black', font=('Arial', 16))
        self._label.pack(anchor='nw', padx=20, pady=20)

    def _on_release(self, event):
        x = self.winfo_pointerx()
        y = self.winfo_pointery()
        try:
            self.on_capture((x, y))
        except Exception:
            pass
        self.destroy()

    def _on_escape(self, event):
        self.destroy()
