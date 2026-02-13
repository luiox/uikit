#include<Windows.h>

//安装键盘钩子
extern "C" __declspec(dllimport) BOOL InstallHook();
//卸载键盘钩子
extern "C" __declspec(dllimport) BOOL UninstallHook();
//钩子处理函数
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);

#pragma comment(lib,"kb.lib")

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPreInstance,LPSTR lpCmdLine,int nCmdShow)
{
	InstallHook();

	while (1);

	return 0;
}