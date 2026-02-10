d:/masm32/bin/ml.exe /c /Zi /coff /I"d:/masm32/include" cdemo.asm
d:/masm32/bin/link.exe /SUBSYSTEM:CONSOLE /OUT:cdemo.exe cdemo.obj d:/masm32/lib/user32.lib d:/masm32/lib/kernel32.lib d:/masm32/lib/gdi32.lib d:/masm32/lib/msvcrt.lib
