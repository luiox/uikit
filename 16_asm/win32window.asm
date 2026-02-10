.386
.model flat, stdcall
option casemap :none
;global START

include windows.inc
include user32.inc
include kernel32.inc
include gdi32.inc

includelib user32.lib
includelib kernel32.lib
includelib gdi32.lib

.const
    ClassName db "MyWindowClass",0
    WindowName db "My Window",0

.data
    ; 数据区

.code
MainProc proc hWin:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
    .if uMsg == WM_CLOSE
        ; 退出
        INVOKE PostQuitMessage, 0
    .endif
    INVOKE DefWindowProc, hWin, uMsg, wParam, lParam
    ret
MainProc endp

START proc
    local @wcx:WNDCLASSEX
    local @hInstance: HINSTANCE
    local @hWnd: HWND
    local @msg: MSG

    ; 初始化窗口类
    mov @wcx.cbSize, sizeof WNDCLASSEX
    mov @wcx.style, CS_HREDRAW or CS_VREDRAW
    mov @wcx.lpfnWndProc, offset MainProc
    mov @wcx.cbClsExtra, 0
    mov @wcx.cbWndExtra, 0
    ; 拿hInstance
    INVOKE GetModuleHandle, NULL
    ; 保存一下
    mov @hInstance, eax
    mov @wcx.hInstance, eax
    ; 拿hIcon
    INVOKE LoadIcon, NULL, IDI_APPLICATION
    mov @wcx.hIcon, eax
    ; 拿hCursor
    INVOKE LoadCursor, NULL, IDC_ARROW
    mov @wcx.hCursor, eax
    ; 拿一个hBrush
    INVOKE GetStockObject, WHITE_BRUSH
    mov @wcx.hbrBackground, eax
    mov @wcx.lpszMenuName, NULL
    mov @wcx.lpszClassName, offset ClassName
    mov @wcx.hIconSm, NULL

    ; 注册窗口类
    INVOKE RegisterClassEx, addr @wcx

    ; 检查返回值是否产生错误
    .if eax == 0
        ret
    .endif

    ; 创建窗口
    INVOKE CreateWindowEx, 
        0,
        offset ClassName, 
        offset WindowName, 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        NULL, 
        NULL, 
        @hInstance, 
        NULL

    mov @hWnd, eax

    ; 显示窗口
    INVOKE ShowWindow, @hWnd, SW_SHOWNORMAL

    ; 消息循环
    .while TRUE
        INVOKE GetMessage, addr @msg, NULL, 0, 0
        .if eax == 0
            .break
        .elseif eax == -1
            ; 退出消息
            .break
        .endif
        INVOKE TranslateMessage, addr @msg
        INVOKE DispatchMessage, addr @msg
    .endw

    ; 退出程序
    INVOKE ExitProcess, 0

    ret

START endp

; 指定程序开始的位置
end START
