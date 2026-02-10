.386 ; 指令集
.model flat, stdcall ; flat 平坦模式（不分段，不使用段寄存器） stdcall 不写就默认使用这个调用约定
OPTION CASEMAP:none ; 区分大小写，让大小写敏感，这个只是伪指令的大小写敏感，针对的是api的名字。

; 栈区
; 如果不定义栈，那么在链接时候指定栈的大小，这样编译器就会自动安排一个栈
;.stack

; 常量区
.const
    ; 因为是字符串因此要'\0'结束。
    MY_MSG db "Hello!", 0
    MY_TITLE db "Title", 0

; 初始化数据区（可以认为全局变量在这里）
.data
    NUM1 db 0 ; 初始化为0，这个0会存放在文件内

; 未初始化数据区
.data?
    NUM2 db ? ; 问号代表随机数，不会占用文件内存

; 函数声明
MessageBoxA proto hWnd:dword, lpText:dword, lpCaption:dword, uType:dword
ExitProcess proto uExitCode:dword

; 代码区
.code
START:
    ; push 0
    ; push 0
    ; push 0
    ; push 0
    ; call MessageBoxA
    ; 等价于上面
    INVOKE MessageBoxA, 0, offset MY_MSG, offset MY_TITLE, 0
    ; 退出程序
    INVOKE ExitProcess, 0
    ret
end START
