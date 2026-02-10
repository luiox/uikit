.386
.model flat, stdcall
option casemap : none

include windows.inc
include user32.inc
include kernel32.inc
include msvcrt.inc

includelib kernel32.lib
includelib user32.lib
includelib msvcrt.lib

.const

.data

.code 
START:

	invoke ExitProcess, 0

end START

