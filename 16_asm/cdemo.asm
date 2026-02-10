.386
.model flat, stdcall
option casemap :none

include windows.inc
include msvcrt.inc
include kernel32.inc
include user32.inc


includelib kernel32.lib
includelib msvcrt.lib

IDD_DIALOG  equ 101

.const
    MyString db "Hello", 0
    MyPause db "pause", 0

.code
DialogProc proc hWnd:DWORD, uMsg:DWORD, wParam:DWORD, lParam:DWORD
    .if uMsg == WM_CLOSE
        INVOKE EndDialog, hWnd, 0
    .endif
    mov eax, TRUE
    ret
DialogProc endp

START proc
    local @hInstance:HINSTANCE

    invoke GetModuleHandle, NULL
    mov @hInstance, eax

    invoke DialogBoxParam, @hInstance, IDD_DIALOG, NULL, offset DialogProc, NULL

    invoke ExitProcess, 0
    ret

START endp

end START
