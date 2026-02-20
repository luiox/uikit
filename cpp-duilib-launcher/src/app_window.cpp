#include "app_window.h"

#include <Windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>

#include <algorithm>
#include <cctype>
#include <filesystem>

#include "file_icon_control.h"

using namespace DuiLib;

namespace {

constexpr UINT kGroupCmdAdd = 1001;
constexpr UINT kGroupCmdRename = 1002;
constexpr UINT kItemCmdAdd = 1101;
constexpr UINT kItemCmdEdit = 1102;
constexpr UINT kItemCmdDelete = 1103;
constexpr UINT kItemCmdMoveBase = 2000;

std::filesystem::path GetAppBaseDir() {
    PWSTR local_app_data = nullptr;
    std::filesystem::path out = std::filesystem::current_path() / "data";
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_app_data)) && local_app_data != nullptr) {
        out = std::filesystem::path(local_app_data) / "nassistant";
        CoTaskMemFree(local_app_data);
    }
    std::error_code ec;
    std::filesystem::create_directories(out, ec);
    return out;
}

std::string ParseIconSource(const backend::LaunchItem& item) {
    if (!item.icon_location.empty()) {
        const auto comma = item.icon_location.find(',');
        if (comma == std::string::npos) {
            return item.icon_location;
        }
        return item.icon_location.substr(0, comma);
    }
    return item.target_path;
}

bool IsSenderFromList(DuiLib::CControlUI* sender, DuiLib::CListUI* list) {
    if (sender == nullptr || list == nullptr) {
        return false;
    }
    if (sender == list) {
        return true;
    }
    DuiLib::CControlUI* walk = sender;
    DuiLib::CControlUI* list_body = list->GetList();
    while (walk != nullptr) {
        if (walk == list || walk == list_body) {
            return true;
        }
        walk = walk->GetParent();
    }
    return false;
}

} // namespace

AppWindow::AppWindow()
    : backend_(GetAppBaseDir(), std::filesystem::current_path()) {}

std::wstring AppWindow::Utf8ToWide(const std::string& text) {
    if (text.empty()) {
        return {};
    }
    const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0);
    if (size <= 0) {
        return std::wstring(text.begin(), text.end());
    }
    std::wstring out(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(), size);
    return out;
}

std::string AppWindow::WideToUtf8(const std::wstring& text) {
    if (text.empty()) {
        return {};
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return std::string(text.begin(), text.end());
    }
    std::string out(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(), size, nullptr, nullptr);
    return out;
}

std::string AppWindow::BasenameNoExt(const std::string& path) {
    std::error_code ec;
    const auto file_name = std::filesystem::path(path).filename().replace_extension("").string();
    if (!ec && !file_name.empty()) {
        return file_name;
    }
    return path;
}

std::string AppWindow::ToLowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool AppWindow::ContainsCaseInsensitive(const std::string& text, const std::string& keyword) const {
    return ToLowerAscii(text).find(ToLowerAscii(keyword)) != std::string::npos;
}

bool AppWindow::IsSearchMode() const {
    return search_mode_;
}

void AppWindow::UpdateSearchUi() {
    if (group_panel_ != nullptr) {
        group_panel_->SetVisible(!search_mode_);
    }
    if (panel_splitter_ != nullptr) {
        panel_splitter_->SetVisible(!search_mode_);
    }
    if (search_input_ != nullptr) {
        search_input_->SetVisible(search_mode_);
        if (search_mode_) {
            search_input_->SetFocus();
        }
    }
    m_pm.NeedUpdate();
}

bool AppWindow::LoadBackendData() {
    std::string error;
    if (!backend_.Load(&error)) {
        status_.Error("load failed: " + error);
        return false;
    }

    RenderGroups();
    if (!group_ids_.empty()) {
        SelectGroupByIndex(0);
    }
    status_.Info("ready");
    return true;
}

void AppWindow::RenderGroups() {
    if (!groups_list_) {
        return;
    }
    groups_list_->RemoveAll();
    group_ids_.clear();

    std::vector<const backend::Group*> groups;
    groups.reserve(backend_.Data().groups.size());
    for (const auto& group : backend_.Data().groups) {
        groups.push_back(&group);
    }
    std::sort(groups.begin(), groups.end(), [](const backend::Group* lhs, const backend::Group* rhs) {
        return lhs->order < rhs->order;
    });

    for (const auto* group : groups) {
        auto* row = new CListLabelElementUI();
        row->SetText(Utf8ToWide(group->name).c_str());
        row->SetFixedHeight(26);
        row->SetAttribute(_T("padding"), _T("8,0,0,0"));
        row->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        groups_list_->Add(row);
        group_ids_.push_back(group->id);
    }
}

void AppWindow::RenderItems() {
    if (!items_list_) {
        return;
    }

    items_list_->RemoveAll();
    item_ids_.clear();
    item_group_ids_.clear();
    selected_item_id_.clear();
    selected_item_group_id_.clear();

    if (search_mode_) {
        std::string keyword;
        if (search_input_ != nullptr) {
            keyword = WideToUtf8(search_input_->GetText().GetData());
        }

        for (const auto& group : backend_.Data().groups) {
            for (const auto& item : group.items) {
                if (item.item_type == "separator") {
                    continue;
                }
                if (!keyword.empty() && !ContainsCaseInsensitive(item.name, keyword)) {
                    continue;
                }

                auto* row = new CListContainerElementUI();
                row->SetFixedHeight(28);
                row->SetAttribute(_T("inset"), _T("8,0,8,0"));
                row->SetAttribute(_T("childpadding"), _T("6"));
                row->SetAttribute(_T("childvalign"), _T("vcenter"));

                auto* icon = new FileIconControl();
                icon->SetFixedWidth(20);
                icon->SetFixedHeight(20);
                icon->SetIconPath(Utf8ToWide(ParseIconSource(item)));
                row->Add(icon);

                auto* name = new CLabelUI();
                name->SetText(Utf8ToWide(item.name + "  [" + group.name + "]").c_str());
                name->SetTextColor(0xFF2E3D4F);
                name->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                row->Add(name);

                items_list_->Add(row);
                item_ids_.push_back(item.id);
                item_group_ids_.push_back(group.id);
            }
        }
        return;
    }

    const backend::Group* group = FindActiveGroup();
    if (!group) {
        return;
    }

    for (const auto& item : group->items) {
        if (item.item_type == "separator") {
            auto* row = new CListLabelElementUI();
            row->SetText(Utf8ToWide(item.name).c_str());
            row->SetFixedHeight(28);
            row->SetAttribute(_T("padding"), _T("8,0,0,0"));
            row->SetTextColor(0xFF7F8A99);
            row->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            items_list_->Add(row);
            item_ids_.push_back(item.id);
            item_group_ids_.push_back(group->id);
            continue;
        }

        auto* row = new CListContainerElementUI();
    row->SetFixedHeight(28);
    row->SetAttribute(_T("inset"), _T("8,0,8,0"));
        row->SetAttribute(_T("childpadding"), _T("6"));
        row->SetAttribute(_T("childvalign"), _T("vcenter"));

        auto* icon = new FileIconControl();
        icon->SetFixedWidth(20);
        icon->SetFixedHeight(20);
        icon->SetIconPath(Utf8ToWide(ParseIconSource(item)));
        row->Add(icon);

        auto* name = new CLabelUI();
        name->SetText(Utf8ToWide(item.name).c_str());
        name->SetTextColor(0xFF2E3D4F);
        name->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        row->Add(name);

        items_list_->Add(row);
        item_ids_.push_back(item.id);
        item_group_ids_.push_back(group->id);
    }
}

void AppWindow::SelectGroupByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(group_ids_.size())) {
        return;
    }
    active_group_id_ = group_ids_[index];
    if (groups_list_) {
        groups_list_->SelectItem(index, false);
    }
    RenderItems();
}

void AppWindow::LaunchSelectedItem() {
    if (selected_item_id_.empty()) {
        status_.Warn("no item selected");
        return;
    }

    const std::string group_id = !selected_item_group_id_.empty() ? selected_item_group_id_ : active_group_id_;
    if (group_id.empty()) {
        status_.Warn("no group selected");
        return;
    }

    std::string error;
    const auto result = backend_.Launch(group_id, selected_item_id_, &error);
    if (!result.ok) {
        status_.Error(error.empty() ? result.message : ("launch failed: " + error));
        return;
    }

    status_.Info(result.message);
    RenderItems();
}

void AppWindow::DeleteSelectedItem() {
    if (selected_item_id_.empty()) {
        status_.Warn("select an item first");
        return;
    }

    const std::string group_id = !selected_item_group_id_.empty() ? selected_item_group_id_ : active_group_id_;
    if (group_id.empty()) {
        status_.Warn("no group selected");
        return;
    }

    std::string error;
    if (!backend_.DeleteItem(group_id, selected_item_id_, &error)) {
        status_.Error("delete item failed: " + error);
        return;
    }

    status_.Info("item deleted");
    RenderItems();
}

CControlUI* AppWindow::BuildRootUi() {
    auto* root = new CVerticalLayoutUI();
    root->SetAttribute(_T("bkcolor"), _T("0xFFF1F3F8"));
    root->SetAttribute(_T("bordercolor"), _T("0xFF9AA4B2"));
    root->SetAttribute(_T("bordersize"), _T("1"));

    auto* topBar = new CHorizontalLayoutUI();
    topBar->SetName(_T("top_bar"));
    topBar->SetFixedHeight(30);
    topBar->SetAttribute(_T("bkcolor"), _T("0xFF3E556B"));
    topBar->SetAttribute(_T("childvalign"), _T("vcenter"));
    topBar->SetAttribute(_T("childpadding"), _T("6"));
    topBar->SetAttribute(_T("inset"), _T("10,0,6,0"));

    auto* title = new CLabelUI();
    title->SetText(_T("nassistant"));
    title->SetTextColor(0xFFFFFFFF);
    title->SetFont(0);
    title->SetFixedWidth(220);
    topBar->Add(title);

    auto* fill = new CControlUI();
    topBar->Add(fill);

    auto* searchBtn = new CButtonUI();
    searchBtn->SetName(_T("searchbtn"));
    searchBtn->SetText(_T("S"));
    searchBtn->SetFixedWidth(24);
    searchBtn->SetFixedHeight(22);
    searchBtn->SetAttribute(_T("normalbkcolor"), _T("0xFF52718C"));
    searchBtn->SetAttribute(_T("hotbkcolor"), _T("0xFF5E7F9C"));
    searchBtn->SetAttribute(_T("pushedbkcolor"), _T("0xFF46627A"));
    searchBtn->SetAttribute(_T("textcolor"), _T("0xFFFFFFFF"));
    searchBtn->SetAttribute(_T("bordercolor"), _T("0x00000000"));
    topBar->Add(searchBtn);

    auto* searchInput = new CEditUI();
    searchInput->SetName(_T("search_input"));
    searchInput->SetVisible(false);
    searchInput->SetFixedWidth(260);
    searchInput->SetFixedHeight(22);
    searchInput->SetAttribute(_T("bordercolor"), _T("0xFF7F8FA0"));
    searchInput->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    searchInput->SetAttribute(_T("textpadding"), _T("6,2,6,2"));
    topBar->Add(searchInput);

    auto* closeBtn = new CButtonUI();
    closeBtn->SetName(_T("closebtn"));
    closeBtn->SetText(_T("X"));
    closeBtn->SetFixedWidth(24);
    closeBtn->SetFixedHeight(22);
    closeBtn->SetAttribute(_T("normalbkcolor"), _T("0xFF7D3A3A"));
    closeBtn->SetAttribute(_T("hotbkcolor"), _T("0xFF944444"));
    closeBtn->SetAttribute(_T("pushedbkcolor"), _T("0xFF6A3030"));
    closeBtn->SetAttribute(_T("textcolor"), _T("0xFFFFFFFF"));
    closeBtn->SetAttribute(_T("bordercolor"), _T("0x00000000"));
    topBar->Add(closeBtn);

    root->Add(topBar);

    auto* body = new CHorizontalLayoutUI();
    body->SetName(_T("body_layout"));
    body->SetAttribute(_T("inset"), _T("6,6,6,6"));
    body->SetAttribute(_T("childpadding"), _T("0"));

    auto* groupPanel = new CVerticalLayoutUI();
    groupPanel->SetName(_T("group_panel"));
    groupPanel->SetFixedWidth(220);
    groupPanel->SetAttribute(_T("bkcolor"), _T("0xFFE8EDF3"));
    groupPanel->SetAttribute(_T("bordercolor"), _T("0xFFB8C3CF"));
    groupPanel->SetAttribute(_T("bordersize"), _T("1"));

    auto* groupTitle = new CLabelUI();
    groupTitle->SetText(_T("Groups"));
    groupTitle->SetFixedHeight(28);
    groupTitle->SetAttribute(_T("padding"), _T("10,7,0,0"));
    groupTitle->SetTextColor(0xFF2A3D52);
    groupTitle->SetFont(0);
    groupPanel->Add(groupTitle);

    auto* groups = new CListUI();
    groups->SetName(_T("groups_list"));
    groups->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    groups->SetAttribute(_T("bordercolor"), _T("0xFFCFD7E0"));
    groups->SetAttribute(_T("bordersize"), _T("1"));
    groups->SetAttribute(_T("inset"), _T("0,0,0,0"));
    groups->SetChildPadding(0);
    groupPanel->Add(groups);

    body->Add(groupPanel);

    auto* splitter = new CControlUI();
    splitter->SetName(_T("panel_splitter"));
    splitter->SetFixedWidth(1);
    splitter->SetAttribute(_T("bkcolor"), _T("0xFFB8C3CF"));
    body->Add(splitter);

    auto* itemPanel = new CVerticalLayoutUI();
    itemPanel->SetName(_T("item_panel"));
    itemPanel->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    itemPanel->SetAttribute(_T("bordercolor"), _T("0xFFB8C3CF"));
    itemPanel->SetAttribute(_T("bordersize"), _T("1"));

    auto* itemTitle = new CLabelUI();
    itemTitle->SetText(_T("Items"));
    itemTitle->SetFixedHeight(28);
    itemTitle->SetAttribute(_T("padding"), _T("10,7,0,0"));
    itemTitle->SetTextColor(0xFF2A3D52);
    itemTitle->SetFont(0);
    itemPanel->Add(itemTitle);

    auto* items = new CListUI();
    items->SetName(_T("items_list"));
    items->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    items->SetAttribute(_T("bordercolor"), _T("0xFFCFD7E0"));
    items->SetAttribute(_T("bordersize"), _T("1"));
    items->SetAttribute(_T("inset"), _T("0,0,0,0"));
    items->SetChildPadding(0);
    itemPanel->Add(items);

    body->Add(itemPanel);
    root->Add(body);

    auto* status = new CLabelUI();
    status->SetName(_T("status_line"));
    status->SetText(_T("Ready"));
    status->SetFixedHeight(22);
    status->SetAttribute(_T("padding"), _T("8,4,0,0"));
    status->SetAttribute(_T("bkcolor"), _T("0xFFE6EBF2"));
    status->SetAttribute(_T("bordercolor"), _T("0xFFB8C3CF"));
    status->SetAttribute(_T("bordersize"), _T("1,1,1,0"));
    status->SetTextColor(0xFF445A72);
    root->Add(status);

    return root;
}

LRESULT AppWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    m_pm.Init(m_hWnd, GetManagerName(), this);
    m_pm.AddPreMessageFilter(this);

    CControlUI* root = BuildRootUi();
    if (root == nullptr) {
        MessageBox(nullptr, _T("Create UI failed"), _T("DuiLib"), MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return 0;
    }

    m_pm.AttachDialog(root);
    m_pm.AddNotifier(this);

    RECT rc_caption{0, 0, 0, 30};
    m_pm.SetCaptionRect(rc_caption);
    RECT rc_sizebox{6, 6, 6, 6};
    m_pm.SetSizeBox(rc_sizebox);

    groups_list_ = static_cast<CListUI*>(m_pm.FindControl(_T("groups_list")));
    items_list_ = static_cast<CListUI*>(m_pm.FindControl(_T("items_list")));
    status_line_ = static_cast<CLabelUI*>(m_pm.FindControl(_T("status_line")));
    group_panel_ = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("group_panel")));
    panel_splitter_ = m_pm.FindControl(_T("panel_splitter"));
    search_input_ = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input")));
    status_.Bind(status_line_);

    DragAcceptFiles(m_hWnd, TRUE);

    LoadBackendData();
    __InitWindow();
    bHandled = TRUE;
    return 0;
}

void AppWindow::Notify(TNotifyUI& msg) {
    if (_tcscmp(msg.sType, DUI_MSGTYPE_CLICK) == 0) {
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("closebtn")) {
            ShowWindow(m_hWnd, SW_HIDE);
            return;
        }
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("searchbtn")) {
            search_mode_ = !search_mode_;
            if (!search_mode_ && search_input_ != nullptr) {
                search_input_->SetText(_T(""));
            }
            UpdateSearchUi();
            RenderItems();
            status_.Info(search_mode_ ? "search mode on" : "search mode off");
            return;
        }
    }

    if (_tcscmp(msg.sType, DUI_MSGTYPE_TEXTCHANGED) == 0 && msg.pSender != nullptr && msg.pSender->GetName() == _T("search_input")) {
        if (search_mode_) {
            RenderItems();
        }
        return;
    }

    if (_tcscmp(msg.sType, DUI_MSGTYPE_ITEMCLICK) == 0 && msg.pSender != nullptr) {
        if (groups_list_ != nullptr && IsSenderFromList(msg.pSender, groups_list_)) {
            SelectGroupByIndex(groups_list_->GetCurSel());
            return;
        }
        if (items_list_ != nullptr && IsSenderFromList(msg.pSender, items_list_)) {
            const int index = items_list_->GetCurSel();
            if (index >= 0 && index < static_cast<int>(item_ids_.size())) {
                selected_item_id_ = item_ids_[index];
                if (index < static_cast<int>(item_group_ids_.size())) {
                    selected_item_group_id_ = item_group_ids_[index];
                }
            }
            return;
        }
    }

    if ((_tcscmp(msg.sType, DUI_MSGTYPE_ITEMDBCLICK) == 0 || _tcscmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE) == 0) &&
        msg.pSender != nullptr && items_list_ != nullptr && IsSenderFromList(msg.pSender, items_list_)) {
        const int index = items_list_->GetCurSel();
        if (index >= 0 && index < static_cast<int>(item_ids_.size())) {
            selected_item_id_ = item_ids_[index];
            if (index < static_cast<int>(item_group_ids_.size())) {
                selected_item_group_id_ = item_group_ids_[index];
            }
            LaunchSelectedItem();
        }
        return;
    }

    WindowImplBase::Notify(msg);
}

bool AppWindow::SelectListRowFromPoint(CListUI* list, const std::vector<std::string>& ids, const POINT& client_point, std::string* selected_id) {
    if (list == nullptr) {
        return false;
    }
    const RECT list_rect = list->GetPos();
    if (!PtInRect(&list_rect, client_point)) {
        return false;
    }

    for (int i = 0; i < list->GetCount(); ++i) {
        CControlUI* item = list->GetItemAt(i);
        if (item == nullptr) {
            continue;
        }
        const RECT row_rect = item->GetPos();
        if (PtInRect(&row_rect, client_point)) {
            list->SelectItem(i, false);
            if (selected_id != nullptr && i >= 0 && i < static_cast<int>(ids.size())) {
                *selected_id = ids[i];
            }
            return true;
        }
    }
    return true;
}

void AppWindow::ShowGroupContextMenu(const POINT& screen_point) {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kGroupCmdAdd, L"Add Group");
    AppendMenuW(menu, MF_STRING, kGroupCmdRename, L"Edit Group Name");

    SetForegroundWindow(m_hWnd);
    const UINT command_id = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, screen_point.x, screen_point.y, 0, m_hWnd, nullptr);
    DestroyMenu(menu);

    if (command_id != 0) {
        ExecuteGroupCommand(command_id);
    }
}

void AppWindow::ShowItemContextMenu(const POINT& screen_point) {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kItemCmdAdd, L"Add Item");
    AppendMenuW(menu, MF_STRING, kItemCmdEdit, L"Edit Item");
    AppendMenuW(menu, MF_STRING, kItemCmdDelete, L"Delete Item");

    HMENU move_menu = CreatePopupMenu();
    for (int i = 0; i < static_cast<int>(group_ids_.size()); ++i) {
        if (group_ids_[i] == active_group_id_) {
            continue;
        }
        const backend::Group* group = nullptr;
        for (const auto& candidate : backend_.Data().groups) {
            if (candidate.id == group_ids_[i]) {
                group = &candidate;
                break;
            }
        }
        if (group != nullptr) {
            AppendMenuW(move_menu, MF_STRING, kItemCmdMoveBase + static_cast<UINT>(i), Utf8ToWide(group->name).c_str());
        }
    }
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(move_menu), L"Move To Group");

    SetForegroundWindow(m_hWnd);
    const UINT command_id = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, screen_point.x, screen_point.y, 0, m_hWnd, nullptr);
    DestroyMenu(menu);

    if (command_id != 0) {
        ExecuteItemCommand(command_id);
    }
}

void AppWindow::ExecuteGroupCommand(UINT command_id) {
    if (command_id == kGroupCmdAdd) {
        std::string error;
        const auto group_id = backend_.AddGroup(GenerateNewGroupName(), &error);
        if (group_id.empty()) {
            status_.Error("add group failed: " + error);
            return;
        }

        RenderGroups();
        for (int i = 0; i < static_cast<int>(group_ids_.size()); ++i) {
            if (group_ids_[i] == group_id) {
                SelectGroupByIndex(i);
                break;
            }
        }
        status_.Info("group added");
        return;
    }

    if (command_id == kGroupCmdRename) {
        const backend::Group* group = FindActiveGroup();
        if (group == nullptr) {
            status_.Warn("no group selected");
            return;
        }
        std::string error;
        const std::string renamed = group->name + " (edited)";
        if (!backend_.RenameGroup(group->id, renamed, &error)) {
            status_.Error("rename group failed: " + error);
            return;
        }
        RenderGroups();
        for (int i = 0; i < static_cast<int>(group_ids_.size()); ++i) {
            if (group_ids_[i] == active_group_id_) {
                groups_list_->SelectItem(i, false);
                break;
            }
        }
        status_.Info("group renamed");
    }
}

void AppWindow::ExecuteItemCommand(UINT command_id) {
    if (command_id == kItemCmdAdd) {
        AddItemFromFile();
        return;
    }
    if (command_id == kItemCmdEdit) {
        EditSelectedItem();
        return;
    }
    if (command_id == kItemCmdDelete) {
        DeleteSelectedItem();
        return;
    }
    if (command_id >= kItemCmdMoveBase) {
        const int group_index = static_cast<int>(command_id - kItemCmdMoveBase);
        if (group_index < 0 || group_index >= static_cast<int>(group_ids_.size())) {
            status_.Warn("invalid target group");
            return;
        }
        MoveSelectedItemToGroup(group_ids_[group_index]);
    }
}

std::wstring AppWindow::PickExecutablePath() const {
    wchar_t file_path[MAX_PATH] = {0};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = file_path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    if (!GetOpenFileNameW(&ofn)) {
        return {};
    }
    return file_path;
}

bool AppWindow::AddItemFromFile() {
    if (active_group_id_.empty()) {
        status_.Warn("select a group first");
        return false;
    }

    const std::wstring file_path = PickExecutablePath();
    if (file_path.empty()) {
        status_.Warn("add item canceled");
        return false;
    }

    backend::ItemInput input;
    input.item_type = std::string("app");
    input.name = BasenameNoExt(WideToUtf8(file_path));
    input.target_path = WideToUtf8(file_path);
    input.icon_location = input.target_path;
    input.arguments.clear();
    input.enabled = true;

    std::string error;
    if (!backend_.UpsertItem(active_group_id_, input, &error)) {
        status_.Error("add item failed: " + error);
        return false;
    }

    RenderItems();
    status_.Info("item added");
    return true;
}

bool AppWindow::EditSelectedItem() {
    const backend::LaunchItem* item = FindSelectedItem();
    if (item == nullptr) {
        status_.Warn("select an item first");
        return false;
    }

    const std::wstring file_path = PickExecutablePath();
    if (file_path.empty()) {
        status_.Warn("edit item canceled");
        return false;
    }

    backend::ItemInput input;
    input.id = item->id;
    input.item_type = item->item_type;
    input.name = BasenameNoExt(WideToUtf8(file_path));
    input.target_path = WideToUtf8(file_path);
    input.icon_location = input.target_path;
    input.arguments = item->arguments;
    input.enabled = item->enabled;

    std::string error;
    const std::string group_id = !selected_item_group_id_.empty() ? selected_item_group_id_ : active_group_id_;
    if (!backend_.UpsertItem(group_id, input, &error)) {
        status_.Error("edit item failed: " + error);
        return false;
    }

    RenderItems();
    status_.Info("item updated");
    return true;
}

bool AppWindow::MoveSelectedItemToGroup(const std::string& target_group_id) {
    if (selected_item_id_.empty()) {
        status_.Warn("select an item first");
        return false;
    }
    const std::string source_group_id = !selected_item_group_id_.empty() ? selected_item_group_id_ : active_group_id_;
    if (source_group_id.empty()) {
        status_.Warn("no source group");
        return false;
    }
    if (target_group_id == source_group_id) {
        status_.Warn("item is already in this group");
        return false;
    }

    std::string error;
    if (!backend_.MoveItem(source_group_id, selected_item_id_, target_group_id, &error)) {
        status_.Error("move item failed: " + error);
        return false;
    }

    RenderGroups();
    RenderItems();
    status_.Info("item moved");
    return true;
}

std::string AppWindow::GenerateNewGroupName() const {
    int index = 1;
    while (true) {
        const std::string candidate = (index == 1) ? "New Group" : ("New Group " + std::to_string(index));
        bool exists = false;
        for (const auto& group : backend_.Data().groups) {
            if (group.name == candidate) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            return candidate;
        }
        ++index;
    }
}

const backend::Group* AppWindow::FindActiveGroup() const {
    for (const auto& group : backend_.Data().groups) {
        if (group.id == active_group_id_) {
            return &group;
        }
    }
    return nullptr;
}

const backend::LaunchItem* AppWindow::FindSelectedItem() const {
    const std::string group_id = !selected_item_group_id_.empty() ? selected_item_group_id_ : active_group_id_;
    const backend::Group* group = nullptr;
    for (const auto& candidate : backend_.Data().groups) {
        if (candidate.id == group_id) {
            group = &candidate;
            break;
        }
    }
    if (group == nullptr) {
        return nullptr;
    }
    for (const auto& item : group->items) {
        if (item.id == selected_item_id_) {
            return &item;
        }
    }
    return nullptr;
}

void AppWindow::HandleFileDrop(HDROP drop_handle) {
    const UINT count = DragQueryFileW(drop_handle, 0xFFFFFFFF, nullptr, 0);
    std::vector<std::string> files;
    files.reserve(count);
    for (UINT index = 0; index < count; ++index) {
        const UINT len = DragQueryFileW(drop_handle, index, nullptr, 0);
        std::wstring path(len + 1, L'\0');
        DragQueryFileW(drop_handle, index, path.data(), len + 1);
        path.resize(len);
        files.push_back(WideToUtf8(path));
    }
    DragFinish(drop_handle);

    if (active_group_id_.empty() && !group_ids_.empty()) {
        active_group_id_ = group_ids_.front();
    }

    std::string error;
    const auto created = backend_.CreateItemsFromDroppedPaths(active_group_id_, files, &error);
    if (!error.empty()) {
        status_.Error("drop import failed: " + error);
    } else if (created > 0) {
        RenderItems();
        status_.Info("imported items: " + std::to_string(created));
    } else {
        status_.Warn("no valid dropped items");
    }
}

LRESULT AppWindow::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (uMsg == WM_LBUTTONDOWN) {
        const int x = static_cast<short>(LOWORD(lParam));
        const int y = static_cast<short>(HIWORD(lParam));
        if (panel_splitter_ != nullptr && group_panel_ != nullptr) {
            RECT rc = panel_splitter_->GetPos();
            rc.left -= 3;
            rc.right += 3;
            POINT pt{x, y};
            if (PtInRect(&rc, pt)) {
                splitter_dragging_ = true;
                splitter_drag_start_x_ = x;
                splitter_start_width_ = group_panel_->GetFixedWidth();
                SetCapture(m_hWnd);
                bHandled = TRUE;
                return 0;
            }
        }
    }

    if (uMsg == WM_MOUSEMOVE && splitter_dragging_ && group_panel_ != nullptr) {
        const int x = static_cast<short>(LOWORD(lParam));
        int next_width = splitter_start_width_ + (x - splitter_drag_start_x_);

        RECT client{};
        GetClientRect(m_hWnd, &client);
        const int total_width = client.right - client.left;
        const int min_group = 80;
        const int min_items = 220;
        const int max_group = (total_width - min_items - 12 > min_group) ? (total_width - min_items - 12) : min_group;

        if (next_width < min_group) {
            next_width = min_group;
        }
        if (next_width > max_group) {
            next_width = max_group;
        }

        group_panel_->SetFixedWidth(next_width);
        m_pm.NeedUpdate();
        bHandled = TRUE;
        return 0;
    }

    if (uMsg == WM_LBUTTONUP && splitter_dragging_) {
        splitter_dragging_ = false;
        ReleaseCapture();
        bHandled = TRUE;
        return 0;
    }

    if (uMsg == WM_CONTEXTMENU) {
        POINT screen_point{};
        if (lParam == static_cast<LPARAM>(-1)) {
            GetCursorPos(&screen_point);
        } else {
            screen_point.x = GET_X_LPARAM(lParam);
            screen_point.y = GET_Y_LPARAM(lParam);
        }
        POINT client_point = screen_point;
        ScreenToClient(m_hWnd, &client_point);

        const bool over_group = SelectListRowFromPoint(groups_list_, group_ids_, client_point, &active_group_id_);
        if (over_group) {
            RenderItems();
            ShowGroupContextMenu(screen_point);
            bHandled = TRUE;
            return 0;
        }

        const bool over_item = SelectListRowFromPoint(items_list_, item_ids_, client_point, &selected_item_id_);
        if (over_item) {
            const int index = items_list_ != nullptr ? items_list_->GetCurSel() : -1;
            if (index >= 0 && index < static_cast<int>(item_group_ids_.size())) {
                selected_item_group_id_ = item_group_ids_[index];
            }
            ShowItemContextMenu(screen_point);
            bHandled = TRUE;
            return 0;
        }
    }

    if (uMsg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            LaunchSelectedItem();
            bHandled = TRUE;
            return 0;
        }
        if (wParam == VK_DELETE) {
            DeleteSelectedItem();
            bHandled = TRUE;
            return 0;
        }
        if (wParam == VK_ESCAPE && search_mode_) {
            search_mode_ = false;
            if (search_input_ != nullptr) {
                search_input_->SetText(_T(""));
            }
            UpdateSearchUi();
            RenderItems();
            bHandled = TRUE;
            return 0;
        }
    }

    if (uMsg == WM_DROPFILES) {
        HandleFileDrop(reinterpret_cast<HDROP>(wParam));
        bHandled = TRUE;
        return 0;
    }

    bHandled = FALSE;
    return 0;
}

LRESULT AppWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    PostQuitMessage(0);
    bHandled = FALSE;
    return 0;
}
