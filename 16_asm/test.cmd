d:/masm32/bin/ml.exe /c /Zi /coff /I"d:/masm32/include" test.asm
d:/masm32/bin/link.exe /SUBSYSTEM:WINDOWS /OUT:test.exe test.obj d:/masm32/lib/user32.lib d:/masm32/lib/kernel32.lib
