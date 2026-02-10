d:/masm32/bin/ml.exe /c /Zi /coff /I"d:/masm32/include" win32window.asm
d:/masm32/bin/link.exe /SUBSYSTEM:WINDOWS /OUT:win32window.exe win32window.obj d:/masm32/lib/user32.lib d:/masm32/lib/kernel32.lib d:/masm32/lib/gdi32.lib
