.386
.model flat, stdcall  ;32 bit memory model
option casemap :none  ;case sensitive

include proj1.inc
include psapi.inc
include msvcrt.inc

includelib comctl32.lib
includelib shell32.lib
includelib msvcrt.lib
includelib psapi.lib

IDD_DIALOG1 EQU 101
IDC_BTN1 EQU 1001

.const
	WND_NAME db "IDA v7.6.210427", 0
	ERROR_MSG1 db "error message1", 0
	ERROR_MSG2 db "error message2", 0
	ERROR_MSG3 db "error message3", 0
	MY_USER32 db "USER32.dll", 0
	MY_MSGBOX db "MeassageBoxA", 0

;########################################################################

.data?
	MyMsg dd ?

.code
start:

	;invoke MessageBox,NULL, NULL, NULL, MB_OK
	invoke GetModuleHandle,NULL
	mov		hInstance,eax

    	invoke InitCommonControls
	invoke DialogBoxParam,hInstance,IDD_DIALOG1,NULL,addr DlgProc,NULL
	invoke ExitProcess,0

;########################################################################

INJECT_CODE_START:
	;int 3

	push ebp
	mov ebp, esp
	push 0
	push 0
	push 0
	push 0
label1:	
	; 如果没有User32，最好是先加载一次
	; invoke LoadLibrary,
	; invoke GetProcAddress,
	mov eax, 12345678h
	call eax
	; call [ebp+8]
	; leave
	; ==
	mov esp, ebp
	pop ebp
	
	;invoke MessageBox,NULL, NULL, NULL, MB_OK
	
	; balance stack
	retn 4
INJECT_CODE_END:

;########################################################################
inject proc
	LOCAL @hwnd:HWND
	LOCAL @dwPid:DWORD
	LOCAL @hProcess:HANDLE
	LOCAL @lpBuf:LPVOID
	LOCAL @dwSize:DWORD
	LOCAL @hUser32:DWORD
	LOCAL @old:DWORD
	LOCAL @hMods[1024]:HMODULE
	LOCAL @cbNeeded:DWORD
	LOCAL @szModName[256]:BYTE
	LOCAL @hRemoteBase:DWORD
	LOCAL @lpLocalAddr:DWORD
	
	; inject
	invoke FindWindow, NULL, offset WND_NAME
	mov @hwnd, eax
	.if eax == NULL
		invoke MessageBox, NULL, offset ERROR_MSG1, NULL, MB_OK
	.endif
	
	invoke GetWindowThreadProcessId, @hwnd, addr @dwPid
	
	invoke OpenProcess, PROCESS_ALL_ACCESS, FALSE, @dwPid
	mov @hProcess, eax
	.if eax == NULL
		invoke MessageBox, NULL, offset ERROR_MSG2, NULL, MB_OK
	.endif
	
	; 1. allocate memory in remote thread
	invoke VirtualAllocEx, @hProcess, NULL, 1000h, MEM_COMMIT, PAGE_EXECUTE_READWRITE
	mov @lpBuf, eax
	.if eax == NULL
		invoke MessageBox, NULL, offset ERROR_MSG3, NULL, MB_OK
	.endif
	
	; 代码重定位
	; 修改内存属性
	invoke VirtualProtect, offset INJECT_CODE_START, 1000h, PAGE_EXECUTE_READWRITE, addr @old
	
	; 遍历模块基址
	invoke EnumProcessModules, @hProcess, addr @hMods, sizeof @hMods, addr @cbNeeded
	mov ebx, 0
	mov edx, @cbNeeded
	; edx / 4
	shr edx, 2
	mov @cbNeeded, edx
	.while ebx < @cbNeeded
		invoke GetModuleFileNameEx, @hProcess, dword ptr [@hMods + ebx * 4], addr @szModName, sizeof @szModName
		invoke crt_strstr, addr @szModName, offset MY_USER32
		; chech if it is User32.dll
		.if eax != 0
			mov eax, dword ptr [@hMods + ebx * 4]
			mov @hRemoteBase, eax
			.break
		.endif
	.endw
	
	invoke LoadLibrary, offset MY_USER32
	mov @hUser32, eax
	
	; get MessageBoxA address
	invoke GetProcAddress, @hUser32, offset MY_MSGBOX
	mov @lpLocalAddr, eax
	; 运行时修改地址
	; remoteBase + (MeassageBoxA - @hUser32)
	sub eax, @hUser32
	add eax, @hRemoteBase

	mov dword ptr [label1 +1], eax
	
	
	; 2. write inject code
	; calc the code size
	mov ebx, offset INJECT_CODE_END
	sub ebx, offset INJECT_CODE_START
	invoke WriteProcessMemory, @hProcess, @lpBuf, offset INJECT_CODE_START, ebx, addr @dwSize
	
	; 3. create remote thread
	invoke CreateRemoteThread, @hProcess, NULL, 0, @lpBuf, NULL, 0, NULL
	
	ret

inject endp

;########################################################################
DlgProc proc hWin:HWND,uMsg:UINT,wParam:WPARAM,lParam:LPARAM

	mov		eax,uMsg
	.if eax==WM_INITDIALOG

	.elseif eax==WM_COMMAND
		; inject button handler
		mov eax, wParam
		.if ax == IDC_BTN1
			invoke inject
		.endif
	.elseif eax == WM_CLOSE
		invoke EndDialog,hWin,0
	.else
		mov		eax,FALSE
		ret
	.endif
	mov		eax,TRUE
	ret

DlgProc endp

end start
