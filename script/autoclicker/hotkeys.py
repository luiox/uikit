"""Hotkey manager providing RegisterHotKey and fallback keyboard bindings for F8 and F9."""
import threading
import keyboard
from ctypes import windll, wintypes, byref
from ctypes import c_short
import time

class HotkeyManager:
    """Manage registration/unregistration of global hotkeys (Windows) and optionally fallback to keyboard module.
    Designed for simple use: register_f8(callback), register_f9(callback) and unregister methods.
    """

    def __init__(self, root, use_register=True):
        self.root = root
        self.user32 = windll.user32
        self.kernel32 = windll.kernel32
        self.listener_thread = None
        self.listener_thread_id = None
        self._registered = {}
        self._callbacks = {}
        self._use_register = use_register
        # Polling fallback state
        self._poll_thread = None
        self._poll_stop = None
        self._poll_keys = {}  # key_name -> callback
        self._poll_prev = {}

    def register_f8(self, callback):
        """Register F8 global hotkey. callback is run in the Tk root thread via root.after(0, callback)."""
        return self.register_hotkey('F8', 1, callback)

    def register_f9(self, callback):
        return self.register_hotkey('F9', 2, callback)

    def register_hotkey(self, key_name, id, callback):
        MOD_NONE = 0x0000
        key_map = {'F8': 0x77, 'F9': 0x78}
        key = key_map.get(key_name)
        if not key:
            raise ValueError('Unknown key')
        # Avoid registering duplicates
        if self._registered.get(id) or self._callbacks.get(id) or self._callbacks.get(f'kb_{key_name}'):
            return True
        if self._use_register:
            try:
                if not self.user32.RegisterHotKey(None, id, MOD_NONE, key):
                    # registration failed
                    raise RuntimeError('RegisterHotKey failed')
                self._registered[id] = True
                self._callbacks[id] = callback
                if not self.listener_thread:
                    self.listener_thread = threading.Thread(target=self._msg_loop, daemon=True)
                    self.listener_thread.start()
                return True
            except Exception:
                # fallback to keyboard
                self._use_register = False
        # fallback to keyboard module (use add_hotkey for better removal)
        def _cb():
            try:
                self.root.after(0, callback)
            except Exception:
                pass
        hk = keyboard.add_hotkey(key_name.lower(), _cb)
        self._callbacks[f'kb_{key_name}'] = hk
        # Also register polling fallback for games that swallow hooks
        try:
            self._register_poll_key(key_name, callback)
        except Exception:
            pass
        return True

    def _start_polling_if_needed(self):
        if self._poll_thread and self._poll_thread.is_alive():
            return
        self._poll_stop = threading.Event()
        self._poll_thread = threading.Thread(target=self._poll_loop, daemon=True)
        self._poll_thread.start()

    def _poll_loop(self):
        # Use GetAsyncKeyState to poll key states
        user32 = self.user32
        VK_MAP = {'F8': 0x77, 'F9': 0x78}
        while not (self._poll_stop and self._poll_stop.is_set()):
            for key_name, cb in list(self._poll_keys.items()):
                vk = VK_MAP.get(key_name)
                if not vk:
                    continue
                try:
                    state = user32.GetAsyncKeyState(vk)
                except Exception:
                    state = 0
                pressed = (state & 0x8000) != 0
                prev = self._poll_prev.get(key_name, False)
                if pressed and not prev:
                    try:
                        self.root.after(0, cb)
                    except Exception:
                        pass
                self._poll_prev[key_name] = pressed
            time.sleep(0.04)

    def _register_poll_key(self, key_name, callback):
        if key_name in self._poll_keys:
            return
        self._poll_keys[key_name] = callback
        self._start_polling_if_needed()

    def _unregister_poll_key(self, key_name):
        if key_name in self._poll_keys:
            del self._poll_keys[key_name]
        if key_name in self._poll_prev:
            del self._poll_prev[key_name]
        # stop thread when no keys
        if not self._poll_keys and self._poll_stop:
            try:
                self._poll_stop.set()
            except Exception:
                pass

    def unregister_f8(self):
        return self.unregister_hotkey(1)

    def unregister_f9(self):
        return self.unregister_hotkey(2)

    def unregister_hotkey(self, id):
        if id in self._registered and self._registered[id]:
            try:
                self.user32.UnregisterHotKey(None, id)
                self._registered[id] = False
            except Exception:
                pass
        # handle keyboard fallback unregister
        kb_key = {1: 'F8', 2: 'F9'}.get(id)
        if kb_key:
            key = f'kb_{kb_key}'
            hk = self._callbacks.get(key)
            if hk is not None:
                try:
                    keyboard.remove_hotkey(hk)
                except Exception:
                    pass
                del self._callbacks[key]
            # unregister polling fallback too
            try:
                self._unregister_poll_key(kb_key)
            except Exception:
                pass
        # remove callback for registered id
        if id in self._callbacks:
            try:
                del self._callbacks[id]
            except Exception:
                pass

    def _msg_loop(self):
        WM_HOTKEY = 0x0312
        try:
            self.listener_thread_id = self.kernel32.GetCurrentThreadId()
        except Exception:
            self.listener_thread_id = None

        msg = wintypes.MSG()
        while True:
            res = self.user32.GetMessageW(byref(msg), None, 0, 0)
            if res == 0:
                break
            if res == -1:
                break
            if msg.message == WM_HOTKEY:
                wParam = msg.wParam
                callback = self._callbacks.get(wParam)
                if callback:
                    try:
                        self.root.after(0, callback)
                    except Exception:
                        pass
            self.user32.TranslateMessage(byref(msg))
            self.user32.DispatchMessageW(byref(msg))

    def close(self):
        # Unregister all
        for key, val in list(self._registered.items()):
            if val:
                try:
                    self.user32.UnregisterHotKey(None, int(key))
                    self._registered[key] = False
                except Exception:
                    pass
        # Post WM_QUIT to listener thread to end msg loop
        if self.listener_thread_id:
            try:
                self.user32.PostThreadMessageW(int(self.listener_thread_id), 0x0012, 0, 0)  # WM_QUIT
            except Exception:
                pass
        # Unregister keyboard fallback hotkeys if present
        for k in list(self._callbacks.keys()):
            if str(k).startswith('kb_'):
                try:
                    keyboard.remove_hotkey(self._callbacks[k])
                except Exception:
                    pass
                del self._callbacks[k]
        # stop poll thread if present
        if self._poll_stop:
            try:
                self._poll_stop.set()
            except Exception:
                pass

    def is_registered(self, key_name):
        """Return True if key_name (e.g., 'F8' or 'F9') is registered either via RegisterHotKey or keyboard fallback."""
        id_map = {'F8': 1, 'F9': 2}
        id = id_map.get(key_name)
        if id and self._registered.get(id):
            return True
        if self._callbacks.get(f'kb_{key_name}'):
            return True
        return False
