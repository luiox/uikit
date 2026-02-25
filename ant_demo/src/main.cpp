#pragma execution_character_set("utf-8")
#include "StdAfx.h"
#include "UIlib.h"
using namespace DuiLib;

class CMainWnd : public DuiLib::WindowImplBase {
public:
    LPCTSTR GetWindowClassName() const override { return _T("AntDemo"); }
    DuiLib::CDuiString GetSkinFile() override { return _T("ant_skin.xml"); }
    // optional: override __InitWindow for post-load initialization
    void __InitWindow() override {
        // initialize controls if needed
    }

    void Notify(TNotifyUI& msg) override;
};

void CMainWnd::Notify(TNotifyUI& msg)
{
    if (msg.sType == DUI_MSGTYPE_CLICK) {
        LPCTSTR name = msg.pSender->GetName();
        if (_tcscmp(name, _T("btn_normal")) == 0) {
            MessageBox(NULL, _T("Normal button clicked"), _T("Notice"), MB_OK);
        } else if (_tcscmp(name, _T("btn_icon")) == 0) {
            MessageBox(NULL, _T("Icon button clicked"), _T("Notice"), MB_OK);
        }
    }
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow)
{
    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetCurrentPath(CPaintManagerUI::GetInstancePath());
    // resources (xml, assets) are stored in "skin" subfolder next to the exe
    CPaintManagerUI::SetResourcePath(
        CPaintManagerUI::GetInstancePath() + _T("\\skin")
    );

    CMainWnd* pFrame = new CMainWnd();
    pFrame->Create(NULL, _T("Ant Demo"), UI_WNDSTYLE_FRAME, 0U);
    pFrame->CenterWindow();
    ::ShowWindow(pFrame->GetHWND(), SW_SHOWDEFAULT);

    CPaintManagerUI::MessageLoop();
    delete pFrame;
    return 0;
}
