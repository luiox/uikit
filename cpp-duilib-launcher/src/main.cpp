#include <windows.h>
#include <UIlib.h>

#include "app_window.h"

using namespace DuiLib;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        return 1;
    }

    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetCurrentPath(CPaintManagerUI::GetInstancePath());
    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

    AppWindow* frame = new AppWindow();
    if (frame == nullptr) {
        CoUninitialize();
        return 1;
    }

    HWND hwnd = frame->Create(nullptr, _T("Poner"), UI_WNDSTYLE_FRAME | WS_SIZEBOX, WS_EX_WINDOWEDGE);
    if (hwnd == nullptr) {
        delete frame;
        CoUninitialize();
        return 2;
    }

    if (!frame->HasRestoredWindowPlacement()) {
        frame->CenterWindow();
    }
    ::ShowWindow(hwnd, frame->ShouldStartMaximized() ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
    ::UpdateWindow(hwnd);

    CPaintManagerUI::MessageLoop();
    CoUninitialize();
    return 0;
}
