#pragma once

#include <UIlib.h>

class MainFrame : public DuiLib::WindowImplBase {
public:
    LPCTSTR GetWindowClassName() const override { return _T("NAssistantMainFrame"); }
    DuiLib::CDuiString GetSkinFile() override { return _T(""); }
    DuiLib::CDuiString GetSkinFolder() { return _T(""); }

    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

protected:
    DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override { return nullptr; }

private:
    DuiLib::CControlUI* BuildRootUi();
};
