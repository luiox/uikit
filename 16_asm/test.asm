.386
.model flat, stdcall
option casemap:none

include windows.inc
include user32.inc
include hello.inc
;includelib user32.lib
 
.data
g_sz db "hello world", 0
.code
START:
invoke MessageBoxA, NULL, offset g_sz, NULL, MB_OK
end START