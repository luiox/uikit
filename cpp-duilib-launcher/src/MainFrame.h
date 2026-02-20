#pragma once

#include <UIlib.h>

#include <string>
#include <vector>

#include "backend.h"

class MainFrame : public DuiLib::WindowImplBase {
public:
    MainFrame();
    LPCTSTR GetWindowClassName() const override { return _T("NAssistantMainFrame"); }
    DuiLib::CDuiString GetSkinFile() override { return _T(""); }
    DuiLib::CDuiString GetSkinFolder() { return _T(""); }

    void Notify(DuiLib::TNotifyUI& msg) override;
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
    LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

protected:
    DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override { return nullptr; }

private:
    DuiLib::CControlUI* BuildRootUi();
    void SetStatus(const std::string& text, bool error = false);
    bool LoadBackendData();
    void RenderGroups();
    void RenderItems();
    void SelectGroupByIndex(int index);
    void LaunchSelectedItem();

    static std::wstring Utf8ToWide(const std::string& text);
    static std::string WideToUtf8(const std::wstring& text);

private:
    backend::LauncherBackend backend_;
    DuiLib::CListUI* groups_list_ = nullptr;
    DuiLib::CListUI* items_list_ = nullptr;
    DuiLib::CLabelUI* status_line_ = nullptr;

    std::vector<std::string> group_ids_;
    std::vector<std::string> item_ids_;
    std::string active_group_id_;
    std::string selected_item_id_;
};
