#include "MainFrame.h"

#include <Windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <algorithm>
#include <filesystem>

using namespace DuiLib;

namespace {

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

} // namespace

MainFrame::MainFrame()
    : backend_(GetAppBaseDir(), std::filesystem::current_path()) {}

std::wstring MainFrame::Utf8ToWide(const std::string& text) {
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

std::string MainFrame::WideToUtf8(const std::wstring& text) {
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

void MainFrame::SetStatus(const std::string& text, bool error) {
    if (!status_line_) {
        return;
    }
    status_line_->SetText(Utf8ToWide(text).c_str());
    status_line_->SetTextColor(error ? 0xFFB00020 : 0xFF445A72);
}

bool MainFrame::LoadBackendData() {
    std::string error;
    if (!backend_.Load(&error)) {
        SetStatus("load failed: " + error, true);
        return false;
    }

    RenderGroups();
    if (!group_ids_.empty()) {
        SelectGroupByIndex(0);
    }
    SetStatus("ready");
    return true;
}

void MainFrame::RenderGroups() {
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
        row->SetFixedHeight(28);
        row->SetAttribute(_T("padding"), _T("8,6,0,0"));
        groups_list_->Add(row);
        group_ids_.push_back(group->id);
    }
}

void MainFrame::RenderItems() {
    if (!items_list_) {
        return;
    }

    items_list_->RemoveAll();
    item_ids_.clear();
    selected_item_id_.clear();

    const backend::Group* group = nullptr;
    for (const auto& it : backend_.Data().groups) {
        if (it.id == active_group_id_) {
            group = &it;
            break;
        }
    }
    if (!group) {
        return;
    }

    for (const auto& item : group->items) {
        auto* row = new CListLabelElementUI();
        row->SetText(Utf8ToWide(item.name).c_str());
        row->SetFixedHeight(30);
        row->SetAttribute(_T("padding"), _T("8,7,0,0"));
        if (item.item_type == "separator") {
            row->SetTextColor(0xFF7F8A99);
            row->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        items_list_->Add(row);
        item_ids_.push_back(item.id);
    }
}

void MainFrame::SelectGroupByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(group_ids_.size())) {
        return;
    }
    active_group_id_ = group_ids_[index];
    if (groups_list_) {
        groups_list_->SelectItem(index, false);
    }
    RenderItems();
}

void MainFrame::LaunchSelectedItem() {
    if (active_group_id_.empty() || selected_item_id_.empty()) {
        return;
    }

    std::string error;
    const auto result = backend_.Launch(active_group_id_, selected_item_id_, &error);
    if (!result.ok) {
        SetStatus(error.empty() ? result.message : error, true);
        return;
    }

    SetStatus(result.message);
    RenderItems();
}

CControlUI* MainFrame::BuildRootUi() {
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
    groups->SetAttribute(_T("inset"), _T("6,0,6,6"));

    auto* groupItem = new CListLabelElementUI();
    groupItem->SetText(_T("Common"));
    groupItem->SetFixedHeight(28);
    groupItem->SetAttribute(_T("padding"), _T("8,6,0,0"));
    groups->Add(groupItem);
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
    items->SetAttribute(_T("inset"), _T("6,0,6,6"));

    auto* item1 = new CListLabelElementUI();
    item1->SetText(_T("Everything"));
    item1->SetFixedHeight(30);
    item1->SetAttribute(_T("padding"), _T("8,7,0,0"));
    items->Add(item1);

    auto* item2 = new CListLabelElementUI();
    item2->SetText(_T("Calibre"));
    item2->SetFixedHeight(30);
    item2->SetAttribute(_T("padding"), _T("8,7,0,0"));
    items->Add(item2);

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

LRESULT MainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
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

    DragAcceptFiles(m_hWnd, TRUE);

    LoadBackendData();
    __InitWindow();
    bHandled = TRUE;
    return 0;
}

void MainFrame::Notify(TNotifyUI& msg) {
    if (_tcscmp(msg.sType, DUI_MSGTYPE_CLICK) == 0) {
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("closebtn")) {
            Close();
            return;
        }
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("searchbtn")) {
            SetStatus("search UI not connected yet");
            return;
        }
    }

    if (_tcscmp(msg.sType, DUI_MSGTYPE_ITEMCLICK) == 0 && msg.pSender != nullptr) {
        if (msg.pSender->GetName() == _T("groups_list") && groups_list_ != nullptr) {
            SelectGroupByIndex(groups_list_->GetCurSel());
            return;
        }
        if (msg.pSender->GetName() == _T("items_list") && items_list_ != nullptr) {
            const int index = items_list_->GetCurSel();
            if (index >= 0 && index < static_cast<int>(item_ids_.size())) {
                selected_item_id_ = item_ids_[index];
            }
            return;
        }
    }

    if ((_tcscmp(msg.sType, DUI_MSGTYPE_ITEMDBCLICK) == 0 || _tcscmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE) == 0) &&
        msg.pSender != nullptr && msg.pSender->GetName() == _T("items_list") && items_list_ != nullptr) {
        const int index = items_list_->GetCurSel();
        if (index >= 0 && index < static_cast<int>(item_ids_.size())) {
            selected_item_id_ = item_ids_[index];
            LaunchSelectedItem();
        }
        return;
    }

    WindowImplBase::Notify(msg);
}

LRESULT MainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
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

    if (uMsg == WM_DROPFILES) {
        HDROP drop = reinterpret_cast<HDROP>(wParam);
        const UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);

        std::vector<std::string> files;
        files.reserve(count);
        for (UINT index = 0; index < count; ++index) {
            const UINT len = DragQueryFileW(drop, index, nullptr, 0);
            std::wstring path(len + 1, L'\0');
            DragQueryFileW(drop, index, path.data(), len + 1);
            path.resize(len);
            files.push_back(WideToUtf8(path));
        }
        DragFinish(drop);

        if (active_group_id_.empty() && !group_ids_.empty()) {
            active_group_id_ = group_ids_.front();
        }

        std::string error;
        const auto created = backend_.CreateItemsFromDroppedPaths(active_group_id_, files, &error);
        if (!error.empty()) {
            SetStatus(error, true);
        } else if (created > 0) {
            RenderItems();
            SetStatus("imported items: " + std::to_string(created));
        } else {
            SetStatus("no valid dropped items", true);
        }

        bHandled = TRUE;
        return 0;
    }

    bHandled = FALSE;
    return 0;
}

LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    PostQuitMessage(0);
    bHandled = FALSE;
    return 0;
}
