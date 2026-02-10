d:/masm32/bin/ml.exe /c /Zi /coff first.asm
d:/masm32/bin/link.exe /SUBSYSTEM:WINDOWS /OUT:first.exe first.obj d:/masm32/lib/user32.lib d:/masm32/lib/kernel32.lib
