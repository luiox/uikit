#include <windows.h>
#include <UIlib.h>

#include "MainFrame.h"

using namespace DuiLib;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        return 1;
    }

    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetCurrentPath(CPaintManagerUI::GetInstancePath());
    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

    MainFrame* frame = new MainFrame();
    if (frame == nullptr) {
        CoUninitialize();
        return 1;
    }

    HWND hwnd = frame->Create(nullptr, _T("NAssistant Launcher (DuiLib)"), UI_WNDSTYLE_FRAME | WS_SIZEBOX, WS_EX_WINDOWEDGE);
    if (hwnd == nullptr) {
        delete frame;
        CoUninitialize();
        return 2;
    }

    frame->CenterWindow();
    frame->ShowWindow(true);

    CPaintManagerUI::MessageLoop();
    CoUninitialize();
    return 0;
}
