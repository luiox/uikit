#include<Windows.h>

//导出函数

HHOOK g_hHook=NULL;

//安装键盘钩子
extern "C" __declspec(dllimport) BOOL InstallHook();
//卸载键盘钩子
extern "C" __declspec(dllimport) BOOL UninstallHook();
//钩子处理函数
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);