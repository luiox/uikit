#pragma once

#include <UIlib.h>

#include <string>
#include <vector>

#include "backend.h"
#include "status_presenter.h"

class AppWindow : public DuiLib::WindowImplBase {
public:
    AppWindow();
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
    bool LoadBackendData();
    void RenderGroups();
    void RenderItems();
    void SelectGroupByIndex(int index);
    void LaunchSelectedItem();
    void DeleteSelectedItem();
    void HandleFileDrop(HDROP drop_handle);
    void UpdateSearchUi();
    bool IsSearchMode() const;

    void ShowGroupContextMenu(const POINT& screen_point);
    void ShowItemContextMenu(const POINT& screen_point);
    void ExecuteGroupCommand(UINT command_id);
    void ExecuteItemCommand(UINT command_id);

    bool AddItemFromFile();
    bool EditSelectedItem();
    bool MoveSelectedItemToGroup(const std::string& target_group_id);

    std::wstring PickExecutablePath() const;
    std::string GenerateNewGroupName() const;
    const backend::Group* FindActiveGroup() const;
    const backend::LaunchItem* FindSelectedItem() const;
    bool SelectListRowFromPoint(DuiLib::CListUI* list, const std::vector<std::string>& ids, const POINT& client_point, std::string* selected_id);

    static std::wstring Utf8ToWide(const std::string& text);
    static std::string WideToUtf8(const std::wstring& text);
    static std::string BasenameNoExt(const std::string& path);
    static std::string ToLowerAscii(std::string value);
    bool ContainsCaseInsensitive(const std::string& text, const std::string& keyword) const;

private:
    backend::LauncherBackend backend_;
    StatusPresenter status_;

    DuiLib::CListUI* groups_list_ = nullptr;
    DuiLib::CListUI* items_list_ = nullptr;
    DuiLib::CLabelUI* status_line_ = nullptr;
    DuiLib::CVerticalLayoutUI* group_panel_ = nullptr;
    DuiLib::CControlUI* panel_splitter_ = nullptr;
    DuiLib::CEditUI* search_input_ = nullptr;

    std::vector<std::string> group_ids_;
    std::vector<std::string> item_ids_;
    std::vector<std::string> item_group_ids_;
    std::string active_group_id_;
    std::string selected_item_id_;
    std::string selected_item_group_id_;

    bool search_mode_ = false;

    bool splitter_dragging_ = false;
    int splitter_drag_start_x_ = 0;
    int splitter_start_width_ = 220;
};
