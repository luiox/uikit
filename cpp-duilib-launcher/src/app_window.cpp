#include "app_window.h"

#include <Windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>

#include "file_icon_control.h"
#include "icons.h"
#include "ui_controls.h"

using namespace DuiLib;

namespace {

constexpr UINT kGroupCmdAdd = 1001;
constexpr UINT kGroupCmdRename = 1002;
constexpr UINT kGroupCmdDelete = 1003;
constexpr UINT kItemCmdAdd = 1101;
constexpr UINT kItemCmdEdit = 1102;
constexpr UINT kItemCmdDelete = 1103;
constexpr UINT kItemCmdRunAs = 1104;
constexpr UINT kItemCmdOpenFolder = 1105;
constexpr UINT kItemCmdShellMenu = 1106;
constexpr UINT kItemCmdCopyPath = 1107;
constexpr UINT kItemCmdMoveBase = 2000;

constexpr UINT kMainCmdNewCustom = 3001;
constexpr UINT kMainCmdSortByName = 3002;
constexpr UINT kMainCmdImportData = 3003;
constexpr UINT kMainCmdExportData = 3004;
constexpr UINT kMainCmdSettings = 3005;
constexpr UINT kMainCmdWebSite = 3006;
constexpr UINT kMainCmdExit = 3007;
constexpr UINT_PTR kUiStateSaveTimerId = 0x4E53;
constexpr UINT_PTR kListDragPollTimerId = 0x4E54;
// UI 状态防抖写入间隔：窗口拖拽/尺寸变化等高频事件会在静默一小段时间后再落盘。
constexpr UINT kUiStateSaveDelayMs = 800;
constexpr bool kDragDebugLog = true;
constexpr int kListDragStartThresholdPx = 2;

struct UiStateSnapshot {
    int splitter_width = 220;
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    int width = 0;
    int height = 0;
    int maximized = 0;
};

void DebugLog(const std::string& text) {
    if (!kDragDebugLog) {
        return;
    }
    std::string line = "[drag] " + text + "\n";
    std::fputs(line.c_str(), stderr);
    std::fflush(stderr);
    ::OutputDebugStringA(line.c_str());
}

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

std::filesystem::path GetEmbeddedIconCacheDir() {
    auto dir = GetAppBaseDir() / "iconlib_cache";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}

std::filesystem::path GetUiStatePath() {
    return GetAppBaseDir() / "ui_state.ini";
}

bool ReadIniInt(const std::filesystem::path& ini_path, const wchar_t* section, const wchar_t* key, int* out) {
    if (out == nullptr) {
        return false;
    }
    wchar_t buffer[64]{};
    const DWORD size = GetPrivateProfileStringW(section, key, L"", buffer, static_cast<DWORD>(std::size(buffer)), ini_path.wstring().c_str());
    if (size == 0) {
        return false;
    }
    wchar_t* end = nullptr;
    const long value = std::wcstol(buffer, &end, 10);
    if (end == buffer) {
        return false;
    }
    *out = static_cast<int>(value);
    return true;
}

bool WriteIniInt(const std::filesystem::path& ini_path, const wchar_t* section, const wchar_t* key, int value) {
    const std::wstring value_text = std::to_wstring(value);
    return ::WritePrivateProfileStringW(section, key, value_text.c_str(), ini_path.wstring().c_str()) != FALSE;
}

bool FlushIniFile(const std::filesystem::path& ini_path) {
    return ::WritePrivateProfileStringW(nullptr, nullptr, nullptr, ini_path.wstring().c_str()) != FALSE;
}

bool WriteUiStateAtomically(const std::filesystem::path& ini_path, const UiStateSnapshot& snapshot) {
    std::filesystem::path tmp_path = ini_path;
    tmp_path += L".tmp";

    std::error_code ec;
    std::filesystem::remove(tmp_path, ec);

    bool ok = true;
    ok = ok && WriteIniInt(tmp_path, L"layout", L"splitter_width", snapshot.splitter_width);
    ok = ok && WriteIniInt(tmp_path, L"window", L"left", snapshot.left);
    ok = ok && WriteIniInt(tmp_path, L"window", L"top", snapshot.top);
    ok = ok && WriteIniInt(tmp_path, L"window", L"right", snapshot.right);
    ok = ok && WriteIniInt(tmp_path, L"window", L"bottom", snapshot.bottom);
    ok = ok && WriteIniInt(tmp_path, L"window", L"width", snapshot.width);
    ok = ok && WriteIniInt(tmp_path, L"window", L"height", snapshot.height);
    ok = ok && WriteIniInt(tmp_path, L"window", L"maximized", snapshot.maximized);

    if (!ok || !FlushIniFile(tmp_path)) {
        std::filesystem::remove(tmp_path, ec);
        return false;
    }

    const std::wstring tmp = tmp_path.wstring();
    const std::wstring dst = ini_path.wstring();
    if (!::MoveFileExW(tmp.c_str(), dst.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        std::filesystem::remove(tmp_path, ec);
        return false;
    }

    return true;
}

void ReplaceAllInPlace(std::string* text, const std::string& from, const std::string& to) {
    if (text == nullptr || from.empty() || from == to) {
        return;
    }
    std::size_t start = 0;
    while ((start = text->find(from, start)) != std::string::npos) {
        text->replace(start, from.length(), to);
        start += to.length();
    }
}

std::string ApplyIconThemeColor(std::string svg_text) {
    static constexpr const char* kThemeColor = "rgb(128,128,128)";
    ReplaceAllInPlace(&svg_text, "currentColor", kThemeColor);
    ReplaceAllInPlace(&svg_text, "#000000", kThemeColor);
    ReplaceAllInPlace(&svg_text, "#000", kThemeColor);
    ReplaceAllInPlace(&svg_text, "black", kThemeColor);
    ReplaceAllInPlace(&svg_text, "rgb(0,0,0)", kThemeColor);
    return svg_text;
}

std::filesystem::path GetThemedIconCacheDir() {
    auto dir = GetEmbeddedIconCacheDir() / "theme_128_128_128";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}

std::filesystem::path BuildThemedIconPath(const std::filesystem::path& source_or_name) {
    const auto stem = source_or_name.stem().string();
    return GetThemedIconCacheDir() / (stem + "_128_128_128.svg");
}

bool WriteThemedSvg(const std::filesystem::path& out_path, const std::string& raw_svg) {
    std::error_code ec;
    std::filesystem::create_directories(out_path.parent_path(), ec);
    std::ofstream stream(out_path, std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) {
        return false;
    }
    const std::string themed = ApplyIconThemeColor(raw_svg);
    stream.write(themed.data(), static_cast<std::streamsize>(themed.size()));
    stream.close();
    return true;
}

std::filesystem::path GetDynamicIconPath(iconlib::Icon icon) {
    const char* rel = iconlib::GetDynamicPath(icon);
    if (rel == nullptr || rel[0] == '\0') {
        return {};
    }
    const auto source = std::filesystem::current_path() / rel;
    if (!std::filesystem::exists(source)) {
        return {};
    }
    std::ifstream stream(source, std::ios::binary);
    if (!stream.is_open()) {
        return source;
    }
    const std::string raw((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    const auto out = BuildThemedIconPath(source);
    if (WriteThemedSvg(out, raw)) {
        return out;
    }
    return source;
}

std::filesystem::path GetEmbeddedIconPath(iconlib::Icon icon) {
    const iconlib::IconAsset* asset = iconlib::FindIcon(icon);
    if (asset == nullptr || asset->svg == nullptr || asset->size == 0) {
        return {};
    }

    const auto out = BuildThemedIconPath(std::filesystem::path(asset->fileName));
    if (!std::filesystem::exists(out)) {
        const std::string raw(asset->svg, asset->svg + asset->size);
        if (!WriteThemedSvg(out, raw)) {
            return {};
        }
    }
    return out;
}

std::filesystem::path ResolveIconPath(iconlib::Icon icon) {
#if ICONLIB_ENABLE_EMBED
    return GetEmbeddedIconPath(icon);
#else
    return GetDynamicIconPath(icon);
#endif
}

iconlib::Icon ResolveTopBarIcon(iconlib::Icon preferred, iconlib::Icon fallback) {
    const auto preferred_path = ResolveIconPath(preferred);
    if (!preferred_path.empty() && std::filesystem::exists(preferred_path)) {
        return preferred;
    }
    const auto fallback_path = ResolveIconPath(fallback);
    if (!fallback_path.empty() && std::filesystem::exists(fallback_path)) {
        return fallback;
    }
    return iconlib::Icon::None;
}

CDuiString MakeSvgImageAttr(iconlib::Icon icon, int draw_px = 16, int box_px = 26) {
    if (icon == iconlib::Icon::None) {
        return {};
    }
    const auto path = ResolveIconPath(icon);
    if (path.empty()) {
        return {};
    }
    if (draw_px <= 0 || box_px <= 0 || draw_px > box_px) {
        draw_px = 16;
        box_px = 26;
    }
    const int offset = (box_px - draw_px) / 2;
    const int left = offset;
    const int top = offset;
    const int right = left + draw_px;
    const int bottom = top + draw_px;

    std::wstring path_w = path.wstring();
    std::replace(path_w.begin(), path_w.end(), L'\\', L'/');
    CDuiString out;
    out.Format(_T("file='%s' dest='%d,%d,%d,%d'"), path_w.c_str(), left, top, right, bottom);
    return out;
}

} // namespace

AppWindow::AppWindow()
    : backend_(GetAppBaseDir(), std::filesystem::current_path()) {
    m_vctStaticName.push_back(_T("apptitlebar"));
}

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

void AppWindow::RestoreUiState() {
    const auto ini_path = GetUiStatePath();

    // 恢复分栏宽度（仅在有效范围内生效）。
    if (group_panel_ != nullptr) {
        int splitter_width = 0;
        if (ReadIniInt(ini_path, L"layout", L"splitter_width", &splitter_width)) {
            if (splitter_width < 80) {
                splitter_width = 80;
            }
            if (splitter_width > 600) {
                splitter_width = 600;
            }
            group_panel_->SetFixedWidth(splitter_width);
        }
    }

    // 兼容两种格式：
    // 1) 老格式：left/top/right/bottom
    // 2) 新格式：left/top/width/height
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;

    const bool has_left = ReadIniInt(ini_path, L"window", L"left", &left);
    const bool has_top = ReadIniInt(ini_path, L"window", L"top", &top);
    const bool has_width = ReadIniInt(ini_path, L"window", L"width", &width);
    const bool has_height = ReadIniInt(ini_path, L"window", L"height", &height);

    if (!(has_left && has_top && has_width && has_height)) {
        int right = 0;
        int bottom = 0;
        if (!ReadIniInt(ini_path, L"window", L"left", &left) ||
            !ReadIniInt(ini_path, L"window", L"top", &top) ||
            !ReadIniInt(ini_path, L"window", L"right", &right) ||
            !ReadIniInt(ini_path, L"window", L"bottom", &bottom)) {
            return;
        }
        width = right - left;
        height = bottom - top;
    }

    if (width < 320 || height < 220) {
        return;
    }

    // 创建后恢复窗口位置与大小，避免每次启动都回到默认尺寸。
    has_restored_window_ = true;
    ::SetWindowPos(m_hWnd, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);

    int maximized = 0;
    if (ReadIniInt(ini_path, L"window", L"maximized", &maximized) && maximized != 0) {
        start_maximized_ = true;
    }
}

void AppWindow::SaveUiState() {
    const auto ini_path = GetUiStatePath();

    UiStateSnapshot snapshot{};

    // 先从当前 UI 组装快照（内存为准），再一次性写盘。
    if (group_panel_ != nullptr) {
        int splitter_width = group_panel_->GetFixedWidth();
        if (splitter_width < 80) {
            splitter_width = 80;
        }
        snapshot.splitter_width = splitter_width;
    }

    WINDOWPLACEMENT placement{};
    placement.length = sizeof(WINDOWPLACEMENT);
    if (!GetWindowPlacement(m_hWnd, &placement)) {
        return;
    }

    // 使用 rcNormalPosition 记录“正常窗口”状态下的几何信息，
    // 这样从最大化退出后仍能恢复到用户期望的普通尺寸。
    const RECT rc = placement.rcNormalPosition;
    snapshot.left = rc.left;
    snapshot.top = rc.top;
    snapshot.right = rc.right;
    snapshot.bottom = rc.bottom;
    snapshot.width = rc.right - rc.left;
    snapshot.height = rc.bottom - rc.top;
    snapshot.maximized = placement.showCmd == SW_SHOWMAXIMIZED ? 1 : 0;

    // 事务性写入：先写临时文件，再原子替换正式配置，避免中途损坏。
    if (!WriteUiStateAtomically(ini_path, snapshot)) {
        ui_state_dirty_ = true;
        ScheduleUiStateSave();
        std::fputs("[ui_state] save failed, retry scheduled\n", stderr);
        std::fflush(stderr);
        ::OutputDebugStringA("[ui_state] save failed, retry scheduled\n");
        return;
    }

    ui_state_dirty_ = false;
}

void AppWindow::ScheduleUiStateSave() {
    if (ui_state_timer_active_) {
        ::KillTimer(m_hWnd, kUiStateSaveTimerId);
    }
    ::SetTimer(m_hWnd, kUiStateSaveTimerId, kUiStateSaveDelayMs, nullptr);
    ui_state_timer_active_ = true;
}

void AppWindow::MarkUiStateDirty() {
    // 状态变更后采用延迟写盘，避免频繁 IO。
    ui_state_dirty_ = true;
    ScheduleUiStateSave();
}

void AppWindow::FlushUiStateIfDirty() {
    if (!ui_state_dirty_) {
        return;
    }
    SaveUiState();
}

bool AppWindow::IsSearchMode() const {
    return search_mode_;
}

void AppWindow::UpdateSearchUi() {
    if (search_bar_ != nullptr) {
        search_bar_->SetVisible(search_mode_);
        search_bar_->SetFixedHeight(search_mode_ ? 30 : 0);
    }
    if (group_panel_ != nullptr) {
        group_panel_->SetVisible(!search_mode_);
    }
    if (panel_splitter_ != nullptr) {
        panel_splitter_->SetVisible(!search_mode_);
    }
    if (search_input_ != nullptr) {
        search_input_->SetVisible(search_mode_);
    }
    m_pm.NeedUpdate();
    if (search_mode_ && search_input_ != nullptr) {
        search_input_->SetFocus();
        const int text_len = search_input_->GetText().GetLength();
        search_input_->SetSel(text_len, text_len);
    }
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
        auto* row = new CListContainerElementUI();
        row->SetFixedHeight(34);

        auto* name = new CLabelUI();
        name->SetText(Utf8ToWide(group->name).c_str());
        name->SetAttribute(_T("padding"), _T("8,0,0,0"));
        name->SetTextColor(0xFF5A5A5A);
        name->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        row->Add(name);

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
                row->SetFixedHeight(34);
                row->SetAttribute(_T("inset"), _T("4,0,4,0"));
                row->SetAttribute(_T("childpadding"), _T("2"));
                row->SetAttribute(_T("childvalign"), _T("vcenter"));

                auto* icon = new FileIconControl();
                icon->SetFixedWidth(26);
                icon->SetFixedHeight(26);
                icon->SetBkColor(0xFFEBEBEB);
                icon->SetIconPath(Utf8ToWide(ParseIconSource(item)));
                row->Add(icon);

                auto* name = new CLabelUI();
                name->SetText(Utf8ToWide(item.name + "  [" + group.name + "]").c_str());
                name->SetTextColor(0xFF5A5A5A);
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
            auto* row = new CListContainerElementUI();
            row->SetFixedHeight(34);

            auto* name = new CLabelUI();
            name->SetText(Utf8ToWide(item.name).c_str());
            name->SetAttribute(_T("padding"), _T("10,0,0,0"));
            name->SetTextColor(0xFF909090);
            name->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            row->Add(name);

            items_list_->Add(row);
            item_ids_.push_back(item.id);
            item_group_ids_.push_back(group->id);
            continue;
        }

        auto* row = new CListContainerElementUI();
        row->SetFixedHeight(34);
        row->SetAttribute(_T("inset"), _T("4,0,4,0"));
        row->SetAttribute(_T("childpadding"), _T("2"));
        row->SetAttribute(_T("childvalign"), _T("vcenter"));

        auto* icon = new FileIconControl();
        icon->SetFixedWidth(26);
        icon->SetFixedHeight(26);
        icon->SetBkColor(0xFFEBEBEB);
        icon->SetIconPath(Utf8ToWide(ParseIconSource(item)));
        row->Add(icon);

        auto* name = new CLabelUI();
        name->SetText(Utf8ToWide(item.name).c_str());
        name->SetTextColor(0xFF5A5A5A);
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
    root->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    root->SetAttribute(_T("bordercolor"), _T("0xFFD2D2D2"));
    root->SetAttribute(_T("bordersize"), _T("1"));
    root->SetAttribute(_T("inset"), _T("0,0,0,0"));
    root->SetAttribute(_T("childpadding"), _T("0"));

    auto* topBar = new appui::TitleBarUI();

    auto* title = new CLabelUI();
    title->SetText(_T("Poner"));
    title->SetTextColor(0xFF5A5A5A);
    title->SetFont(0);
    title->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    topBar->Add(title);

    auto* fill = new CControlUI();
    topBar->Add(fill);

    auto* searchBtn = new appui::IconButtonUI();
    searchBtn->SetName(_T("searchbtn"));
    const auto search_icon = ResolveTopBarIcon(iconlib::Icon::Search, iconlib::Icon::Search);
    const CDuiString search_img_n = MakeSvgImageAttr(search_icon);
    searchBtn->SetSvgImage(search_img_n);
    topBar->Add(searchBtn);

    auto* menuBtn = new appui::IconButtonUI();
    menuBtn->SetName(_T("menubtn"));
    const auto menu_icon = ResolveTopBarIcon(iconlib::Icon::Menu, iconlib::Icon::Menu);
    const CDuiString menu_img_n = MakeSvgImageAttr(menu_icon);
    menuBtn->SetSvgImage(menu_img_n);
    topBar->Add(menuBtn);

    auto* closeBtn = new appui::IconButtonUI();
    closeBtn->SetName(_T("closebtn"));
    const auto close_icon = ResolveTopBarIcon(iconlib::Icon::Close, iconlib::Icon::Clear);
    const CDuiString exit_img_n = MakeSvgImageAttr(close_icon);
    closeBtn->SetSvgImage(exit_img_n);
    topBar->Add(closeBtn);

    root->Add(topBar);

    auto* searchBar = new CHorizontalLayoutUI();
    searchBar->SetName(_T("search_bar"));
    searchBar->SetVisible(false);
    searchBar->SetFixedHeight(0);
    searchBar->SetAttribute(_T("inset"), _T("0,0,0,0"));
    searchBar->SetAttribute(_T("childpadding"), _T("0"));
    searchBar->SetAttribute(_T("bkcolor"), _T("0xFFD2D2D2"));

    auto* searchInput = new appui::SearchBoxUI();
    searchInput->SetName(_T("search_input"));
    searchInput->SetVisible(false);
    searchBar->Add(searchInput);

    root->Add(searchBar);

    auto* body = new CHorizontalLayoutUI();
    body->SetName(_T("body_layout"));
    body->SetAttribute(_T("inset"), _T("0,0,0,0"));
    body->SetAttribute(_T("childpadding"), _T("0"));

    auto* groupPanel = new CVerticalLayoutUI();
    groupPanel->SetName(_T("group_panel"));
    groupPanel->SetFixedWidth(220);
    groupPanel->SetAttribute(_T("bkcolor"), _T("0xFFE6E6E6"));
    groupPanel->SetAttribute(_T("bordercolor"), _T("0xFFD2D2D2"));
    groupPanel->SetAttribute(_T("bordersize"), _T("0"));
    groupPanel->SetAttribute(_T("inset"), _T("0,0,0,0"));
    groupPanel->SetAttribute(_T("childpadding"), _T("0"));

    auto* groups = new appui::GroupListUI();
    groups->SetName(_T("groups_list"));
    groupPanel->Add(groups);

    body->Add(groupPanel);

    auto* splitter = new CControlUI();
    splitter->SetName(_T("panel_splitter"));
    splitter->SetFixedWidth(1);
    splitter->SetAttribute(_T("bkcolor"), _T("0xFFD2D2D2"));
    body->Add(splitter);

    auto* itemPanel = new CVerticalLayoutUI();
    itemPanel->SetName(_T("item_panel"));
    itemPanel->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    itemPanel->SetAttribute(_T("bordercolor"), _T("0xFFB8C3CF"));
    itemPanel->SetAttribute(_T("bordersize"), _T("0"));
    itemPanel->SetAttribute(_T("inset"), _T("0,0,0,0"));
    itemPanel->SetAttribute(_T("childpadding"), _T("0"));

    auto* items = new appui::ItemListUI();
    items->SetName(_T("items_list"));
    itemPanel->Add(items);

    body->Add(itemPanel);
    root->Add(body);

    auto* status = new CLabelUI();
    status->SetName(_T("status_line"));
    status->SetText(_T("Ready"));
    status->SetFixedHeight(26);
    status->SetAttribute(_T("padding"), _T("10,6,0,0"));
    status->SetAttribute(_T("bkcolor"), _T("0xFFF8F8F8"));
    status->SetAttribute(_T("bordercolor"), _T("0xFFDCDCDC"));
    status->SetAttribute(_T("bordersize"), _T("1,1,1,0"));
    status->SetTextColor(0xFF5A5A5A);
    root->Add(status);

    auto* groupDialog = new CVerticalLayoutUI();
    groupDialog->SetName(_T("group_dialog"));
    groupDialog->SetVisible(false);
    groupDialog->SetFloat(true);
    groupDialog->SetFloatAlign(DT_CENTER | DT_VCENTER);
    groupDialog->SetFixedWidth(360);
    groupDialog->SetFixedHeight(150);
    groupDialog->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    groupDialog->SetAttribute(_T("bordercolor"), _T("0xFFD2D2D2"));
    groupDialog->SetAttribute(_T("bordersize"), _T("1"));
    groupDialog->SetAttribute(_T("inset"), _T("12,10,12,10"));
    groupDialog->SetAttribute(_T("childpadding"), _T("8"));

    auto* groupDialogTitle = new CLabelUI();
    groupDialogTitle->SetName(_T("group_dialog_title"));
    groupDialogTitle->SetText(_T("Add Group"));
    groupDialogTitle->SetFixedHeight(24);
    groupDialogTitle->SetTextColor(0xFF5A5A5A);
    groupDialogTitle->SetFont(0);
    groupDialog->Add(groupDialogTitle);

    auto* groupDialogInput = new CEditUI();
    groupDialogInput->SetName(_T("group_dialog_input"));
    groupDialogInput->SetFixedHeight(28);
    groupDialogInput->SetAttribute(_T("bordercolor"), _T("0xFFD2D2D2"));
    groupDialogInput->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    groupDialogInput->SetAttribute(_T("textpadding"), _T("6,3,6,3"));
    groupDialog->Add(groupDialogInput);

    auto* actions = new CHorizontalLayoutUI();
    actions->SetAttribute(_T("childpadding"), _T("8"));
    actions->SetAttribute(_T("childalign"), _T("right"));

    auto* okButton = new CButtonUI();
    okButton->SetName(_T("group_dialog_ok"));
    okButton->SetText(_T("OK"));
    okButton->SetFixedWidth(88);
    okButton->SetFixedHeight(28);
    okButton->SetAttribute(_T("normalbkcolor"), _T("0xFFE6E6E6"));
    okButton->SetAttribute(_T("hotbkcolor"), _T("0xFFD5D5D5"));
    okButton->SetAttribute(_T("pushedbkcolor"), _T("0xFFD5D5D5"));
    okButton->SetAttribute(_T("textcolor"), _T("0xFF5A5A5A"));
    okButton->SetAttribute(_T("bordercolor"), _T("0x00000000"));
    actions->Add(okButton);

    auto* cancelButton = new CButtonUI();
    cancelButton->SetName(_T("group_dialog_cancel"));
    cancelButton->SetText(_T("Cancel"));
    cancelButton->SetFixedWidth(88);
    cancelButton->SetFixedHeight(28);
    cancelButton->SetAttribute(_T("normalbkcolor"), _T("0xFFE6E6E6"));
    cancelButton->SetAttribute(_T("hotbkcolor"), _T("0xFFD5D5D5"));
    cancelButton->SetAttribute(_T("pushedbkcolor"), _T("0xFFD5D5D5"));
    cancelButton->SetAttribute(_T("textcolor"), _T("0xFF5A5A5A"));
    cancelButton->SetAttribute(_T("bordercolor"), _T("0x00000000"));
    actions->Add(cancelButton);

    groupDialog->Add(actions);
    root->Add(groupDialog);

    return root;
}

LRESULT AppWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    m_pm.Init(m_hWnd, GetManagerName(), this);
    m_pm.AddFont(1, _T("微软雅黑"), 16, false, false, false);

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
    search_bar_ = m_pm.FindControl(_T("search_bar"));
    group_panel_ = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("group_panel")));
    panel_splitter_ = m_pm.FindControl(_T("panel_splitter"));
    search_input_ = static_cast<CEditUI*>(m_pm.FindControl(_T("search_input")));
    group_dialog_ = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("group_dialog")));
    group_dialog_title_ = static_cast<CLabelUI*>(m_pm.FindControl(_T("group_dialog_title")));
    group_dialog_input_ = static_cast<CEditUI*>(m_pm.FindControl(_T("group_dialog_input")));
    status_.Bind(status_line_);

    RestoreUiState();

    DragAcceptFiles(m_hWnd, TRUE);

    LoadBackendData();
    __InitWindow();
    bHandled = TRUE;
    return 0;
}

LRESULT AppWindow::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) {
    BOOL custom_handled = FALSE;
    const LRESULT custom_result = HandleCustomMessage(uMsg, wParam, lParam, custom_handled);
    if (custom_handled) {
        bHandled = true;
        return custom_result;
    }
    return WindowImplBase::MessageHandler(uMsg, wParam, lParam, bHandled);
}

void AppWindow::Notify(TNotifyUI& msg) {
    if (_tcscmp(msg.sType, DUI_MSGTYPE_CLICK) == 0) {
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("group_dialog_ok")) {
            ConfirmGroupDialog();
            return;
        }
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("group_dialog_cancel")) {
            CloseGroupDialog();
            return;
        }
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("closebtn")) {
            ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
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
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("menubtn")) {
            RECT rc{};
            GetWindowRect(m_hWnd, &rc);
            POINT menu_point{rc.left + 12, rc.top + 35};
            ShowMainContextMenu(menu_point);
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
    AppendMenuW(menu, MF_STRING, kGroupCmdDelete, L"Delete Group");

    SetForegroundWindow(m_hWnd);
    const UINT command_id = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, screen_point.x, screen_point.y, 0, m_hWnd, nullptr);
    DestroyMenu(menu);

    if (command_id != 0) {
        ExecuteGroupCommand(command_id);
    }
}

void AppWindow::ShowItemContextMenu(const POINT& screen_point) {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kItemCmdRunAs, L"Run as administrator");
    AppendMenuW(menu, MF_STRING, kItemCmdOpenFolder, L"Open file location");
    AppendMenuW(menu, MF_STRING, kItemCmdShellMenu, L"Explorer menu");
    AppendMenuW(menu, MF_STRING, kItemCmdCopyPath, L"Copy full path");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
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

void AppWindow::ShowMainContextMenu(const POINT& screen_point) {
    HMENU menu = CreatePopupMenu();
    HMENU new_menu = CreatePopupMenu();

    AppendMenuW(new_menu, MF_STRING, kMainCmdNewCustom, L"Custom");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(new_menu), L"New Item");
    AppendMenuW(menu, MF_STRING, kMainCmdSortByName, L"Sort By Name");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kMainCmdImportData, L"Import Data");
    AppendMenuW(menu, MF_STRING, kMainCmdExportData, L"Export Data");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kMainCmdSettings, L"Settings");
    AppendMenuW(menu, MF_STRING, kMainCmdWebSite, L"Website");
    AppendMenuW(menu, MF_STRING, kMainCmdExit, L"Exit");

    SetForegroundWindow(m_hWnd);
    const UINT command_id = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, screen_point.x, screen_point.y, 0, m_hWnd, nullptr);
    DestroyMenu(menu);

    if (command_id != 0) {
        ExecuteMainCommand(command_id);
    }
}

void AppWindow::ExecuteMainCommand(UINT command_id) {
    switch (command_id) {
    case kMainCmdNewCustom:
        AddItemFromFile();
        return;
    case kMainCmdSortByName:
        status_.Warn("sort by name is not implemented yet");
        return;
    case kMainCmdImportData:
        status_.Warn("import data is not implemented yet");
        return;
    case kMainCmdExportData:
        status_.Warn("export data is not implemented yet");
        return;
    case kMainCmdSettings:
        status_.Warn("settings window is not implemented yet");
        return;
    case kMainCmdWebSite:
        ShellExecuteW(nullptr, L"open", L"https://www.52pojie.cn/?Poner", nullptr, nullptr, SW_SHOWNORMAL);
        return;
    case kMainCmdExit:
        ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        return;
    default:
        return;
    }
}

void AppWindow::ExecuteGroupCommand(UINT command_id) {
    if (command_id == kGroupCmdAdd) {
        OpenGroupDialog(false, std::string());
        return;
    }

    if (command_id == kGroupCmdRename) {
        const backend::Group* group = FindActiveGroup();
        if (group == nullptr) {
            status_.Warn("no group selected");
            return;
        }
        OpenGroupDialog(true, group->id);
        return;
    }

    if (command_id == kGroupCmdDelete) {
        DeleteActiveGroup();
    }
}

void AppWindow::OpenGroupDialog(bool rename_mode, const std::string& group_id) {
    if (group_dialog_ == nullptr || group_dialog_input_ == nullptr || group_dialog_title_ == nullptr) {
        status_.Error("group dialog is not available");
        return;
    }

    group_dialog_rename_mode_ = rename_mode;
    group_dialog_group_id_ = group_id;

    if (rename_mode) {
        const backend::Group* group = nullptr;
        for (const auto& candidate : backend_.Data().groups) {
            if (candidate.id == group_id) {
                group = &candidate;
                break;
            }
        }
        if (group == nullptr) {
            status_.Warn("group not found");
            return;
        }
        group_dialog_title_->SetText(_T("Rename Group"));
        group_dialog_input_->SetText(Utf8ToWide(group->name).c_str());
    } else {
        group_dialog_title_->SetText(_T("Add Group"));
        group_dialog_input_->SetText(_T(""));
    }

    group_dialog_->SetVisible(true);
    group_dialog_input_->SetFocus();
    m_pm.NeedUpdate();
}

void AppWindow::CloseGroupDialog() {
    if (group_dialog_ == nullptr) {
        return;
    }
    group_dialog_->SetVisible(false);
    group_dialog_group_id_.clear();
    group_dialog_rename_mode_ = false;
    m_pm.NeedUpdate();
}

void AppWindow::ConfirmGroupDialog() {
    if (group_dialog_input_ == nullptr) {
        return;
    }

    const std::string name = WideToUtf8(group_dialog_input_->GetText().GetData());
    std::string trimmed = name;
    trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back()))) {
        trimmed.pop_back();
    }

    if (trimmed.empty()) {
        status_.Warn("group name cannot be empty");
        return;
    }

    std::string error;
    if (group_dialog_rename_mode_) {
        if (!backend_.RenameGroup(group_dialog_group_id_, trimmed, &error)) {
            status_.Error("rename group failed: " + error);
            return;
        }
        status_.Info("group renamed");
    } else {
        const std::string created_id = backend_.AddGroup(trimmed, &error);
        if (created_id.empty()) {
            status_.Error("add group failed: " + error);
            return;
        }
        active_group_id_ = created_id;
        status_.Info("group added");
    }

    CloseGroupDialog();
    RenderGroups();
    for (int i = 0; i < static_cast<int>(group_ids_.size()); ++i) {
        if (group_ids_[i] == active_group_id_) {
            SelectGroupByIndex(i);
            break;
        }
    }
}

void AppWindow::ExecuteItemCommand(UINT command_id) {
    if (command_id == kItemCmdRunAs) {
        RunSelectedItemAsAdmin();
        return;
    }
    if (command_id == kItemCmdOpenFolder) {
        OpenSelectedItemFolder();
        return;
    }
    if (command_id == kItemCmdShellMenu) {
        ShowSelectedItemShellMenu();
        return;
    }
    if (command_id == kItemCmdCopyPath) {
        CopySelectedItemPath();
        return;
    }
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

bool AppWindow::DeleteActiveGroup() {
    const backend::Group* active_group = FindActiveGroup();
    if (active_group == nullptr) {
        status_.Warn("select a group first");
        return false;
    }

    std::string target_group_id;
    for (const auto& group_id : group_ids_) {
        if (group_id != active_group->id) {
            target_group_id = group_id;
            break;
        }
    }
    if (target_group_id.empty()) {
        status_.Warn("cannot delete the last group");
        return false;
    }

    const int confirmed = MessageBoxW(m_hWnd,
        L"The selected group will be deleted and its items moved to another group. Continue?",
        L"Delete Group",
        MB_ICONQUESTION | MB_YESNO);
    if (confirmed != IDYES) {
        status_.Warn("delete group canceled");
        return false;
    }

    std::string error;
    if (!backend_.DeleteGroup(active_group->id, target_group_id, &error)) {
        status_.Error("delete group failed: " + error);
        return false;
    }

    active_group_id_ = target_group_id;
    selected_item_id_.clear();
    selected_item_group_id_.clear();
    RenderGroups();
    for (int i = 0; i < static_cast<int>(group_ids_.size()); ++i) {
        if (group_ids_[i] == active_group_id_) {
            SelectGroupByIndex(i);
            break;
        }
    }
    status_.Info("group deleted");
    return true;
}

bool AppWindow::RunSelectedItemAsAdmin() {
    const backend::LaunchItem* item = FindSelectedItem();
    if (item == nullptr) {
        status_.Warn("select an item first");
        return false;
    }
    if (item->item_type == "separator") {
        status_.Warn("separator item cannot be launched");
        return false;
    }

    const std::wstring target_w = Utf8ToWide(item->target_path);
    const std::wstring args_w = Utf8ToWide(item->arguments);
    HINSTANCE instance = ShellExecuteW(
        m_hWnd,
        L"runas",
        target_w.c_str(),
        args_w.empty() ? nullptr : args_w.c_str(),
        nullptr,
        SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(instance) <= 32) {
        status_.Error("run as administrator failed");
        return false;
    }

    status_.Info("started as administrator");
    return true;
}

bool AppWindow::OpenSelectedItemFolder() {
    const backend::LaunchItem* item = FindSelectedItem();
    if (item == nullptr) {
        status_.Warn("select an item first");
        return false;
    }
    if (item->target_path.empty()) {
        status_.Warn("target path is empty");
        return false;
    }

    PIDLIST_ABSOLUTE pidl = ILCreateFromPathW(Utf8ToWide(item->target_path).c_str());
    if (pidl == nullptr) {
        status_.Error("open file location failed");
        return false;
    }
    const HRESULT hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
    ILFree(pidl);
    if (FAILED(hr)) {
        status_.Error("open file location failed");
        return false;
    }

    status_.Info("opened file location");
    return true;
}

bool AppWindow::ShowSelectedItemShellMenu() {
    const backend::LaunchItem* item = FindSelectedItem();
    if (item == nullptr) {
        status_.Warn("select an item first");
        return false;
    }
    if (item->target_path.empty()) {
        status_.Warn("target path is empty");
        return false;
    }

    const std::wstring path_w = Utf8ToWide(item->target_path);
    HINSTANCE instance = ShellExecuteW(m_hWnd, L"properties", path_w.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(instance) <= 32) {
        status_.Error("open explorer menu failed");
        return false;
    }

    status_.Info("opened explorer properties");
    return true;
}

bool AppWindow::CopySelectedItemPath() {
    const backend::LaunchItem* item = FindSelectedItem();
    if (item == nullptr) {
        status_.Warn("select an item first");
        return false;
    }
    if (item->target_path.empty()) {
        status_.Warn("target path is empty");
        return false;
    }

    const std::wstring text = Utf8ToWide(item->target_path);
    if (!OpenClipboard(m_hWnd)) {
        status_.Error("copy path failed");
        return false;
    }

    EmptyClipboard();
    const std::size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL buffer = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (buffer == nullptr) {
        CloseClipboard();
        status_.Error("copy path failed");
        return false;
    }
    void* ptr = GlobalLock(buffer);
    memcpy(ptr, text.c_str(), bytes);
    GlobalUnlock(buffer);
    SetClipboardData(CF_UNICODETEXT, buffer);
    CloseClipboard();

    status_.Info("path copied");
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

int AppWindow::HitTestListIndex(CListUI* list, const POINT& client_point) const {
    if (list == nullptr) {
        return -1;
    }
    const RECT list_rect = list->GetPos();
    if (!PtInRect(&list_rect, client_point)) {
        return -1;
    }

    for (int i = 0; i < list->GetCount(); ++i) {
        CControlUI* item = list->GetItemAt(i);
        if (item == nullptr || !item->IsVisible()) {
            continue;
        }
        const RECT row_rect = item->GetPos();
        if (PtInRect(&row_rect, client_point)) {
            return i;
        }
    }
    return -1;
}

void AppWindow::ResetListDragState() {
    if (list_drag_polling_) {
        ::KillTimer(m_hWnd, kListDragPollTimerId);
        list_drag_polling_ = false;
    }
    drag_list_kind_ = DragListKind::None;
    list_drag_prepared_ = false;
    list_dragging_ = false;
    list_drag_from_index_ = -1;
    list_drag_hover_index_ = -1;
    list_drag_down_point_.x = 0;
    list_drag_down_point_.y = 0;
}

bool AppWindow::CommitListDragReorder() {
    if (list_drag_from_index_ < 0 || list_drag_hover_index_ < 0 || list_drag_from_index_ == list_drag_hover_index_) {
        DebugLog("commit skipped from=" + std::to_string(list_drag_from_index_) + " to=" + std::to_string(list_drag_hover_index_));
        return false;
    }

    std::string error;
    if (drag_list_kind_ == DragListKind::Groups) {
        if (list_drag_from_index_ >= static_cast<int>(group_ids_.size()) || list_drag_hover_index_ >= static_cast<int>(group_ids_.size())) {
            return false;
        }

        const std::string dragged_group_id = group_ids_[list_drag_from_index_];
        DebugLog("reorder groups request id=" + dragged_group_id + " from=" + std::to_string(list_drag_from_index_) + " to=" + std::to_string(list_drag_hover_index_));
        if (!backend_.ReorderGroup(dragged_group_id, list_drag_hover_index_, &error)) {
            status_.Error("reorder group failed: " + error);
            return false;
        }

        RenderGroups();
        SelectGroupByIndex(list_drag_hover_index_);
        m_pm.NeedUpdate();
        DebugLog("commit groups from=" + std::to_string(list_drag_from_index_) + " to=" + std::to_string(list_drag_hover_index_));
        status_.Info("group reordered");
        return true;
    }

    if (drag_list_kind_ == DragListKind::Items) {
        if (search_mode_) {
            status_.Warn("item reorder is disabled in search mode");
            return false;
        }
        if (list_drag_from_index_ >= static_cast<int>(item_ids_.size()) || list_drag_hover_index_ >= static_cast<int>(item_ids_.size())) {
            return false;
        }
        if (active_group_id_.empty()) {
            return false;
        }

        const std::string dragged_item_id = item_ids_[list_drag_from_index_];
        DebugLog("reorder items request id=" + dragged_item_id + " from=" + std::to_string(list_drag_from_index_) + " to=" + std::to_string(list_drag_hover_index_));
        if (!backend_.ReorderItemInGroup(active_group_id_, dragged_item_id, list_drag_hover_index_, &error)) {
            status_.Error("reorder item failed: " + error);
            return false;
        }

        RenderItems();
        m_pm.NeedUpdate();
        if (items_list_ != nullptr) {
            items_list_->SelectItem(list_drag_hover_index_, false);
        }
        if (list_drag_hover_index_ >= 0 && list_drag_hover_index_ < static_cast<int>(item_ids_.size())) {
            selected_item_id_ = item_ids_[list_drag_hover_index_];
            if (list_drag_hover_index_ < static_cast<int>(item_group_ids_.size())) {
                selected_item_group_id_ = item_group_ids_[list_drag_hover_index_];
            }
        }
        DebugLog("commit items from=" + std::to_string(list_drag_from_index_) + " to=" + std::to_string(list_drag_hover_index_));
        status_.Info("item reordered");
        return true;
    }

    return false;
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
    auto normalize_point = [&](int x, int y) {
        POINT pt{x, y};
        RECT client{};
        ::GetClientRect(m_hWnd, &client);
        if (!PtInRect(&client, pt)) {
            POINT screen_pt{x, y};
            ::ScreenToClient(m_hWnd, &screen_pt);
            if (PtInRect(&client, screen_pt)) {
                pt = screen_pt;
            }
        }
        return pt;
    };

    if (uMsg == WM_GETMINMAXINFO) {
        // 限制最小窗口尺寸，避免顶部按钮/布局被挤压不可见。
        auto* info = reinterpret_cast<MINMAXINFO*>(lParam);
        if (info != nullptr) {
            if (info->ptMinTrackSize.x < 420) {
                info->ptMinTrackSize.x = 420;
            }
            if (info->ptMinTrackSize.y < 280) {
                info->ptMinTrackSize.y = 280;
            }
            bHandled = TRUE;
            return 0;
        }
    }

    if (uMsg == WM_LBUTTONDOWN) {
        const int x = static_cast<short>(LOWORD(lParam));
        const int y = static_cast<short>(HIWORD(lParam));
        const POINT pt = normalize_point(x, y);

        // 记录列表拖拽候选（按下后超过阈值才真正进入拖拽）。
        ResetListDragState();
        const int group_index = HitTestListIndex(groups_list_, pt);
        if (group_index >= 0) {
            drag_list_kind_ = DragListKind::Groups;
            list_drag_prepared_ = true;
            list_drag_down_point_ = pt;
            list_drag_from_index_ = group_index;
            list_drag_hover_index_ = group_index;
            SetCapture(m_hWnd);
            ::SetTimer(m_hWnd, kListDragPollTimerId, 16, nullptr);
            list_drag_polling_ = true;
        } else {
            const int item_index = HitTestListIndex(items_list_, pt);
            if (item_index >= 0 && !search_mode_) {
                drag_list_kind_ = DragListKind::Items;
                list_drag_prepared_ = true;
                list_drag_down_point_ = pt;
                list_drag_from_index_ = item_index;
                list_drag_hover_index_ = item_index;
                SetCapture(m_hWnd);
                ::SetTimer(m_hWnd, kListDragPollTimerId, 16, nullptr);
                list_drag_polling_ = true;
            }
        }

        DebugLog("down g=" + std::to_string(group_index) + " i=" + std::to_string(HitTestListIndex(items_list_, pt)));

        if (panel_splitter_ != nullptr && group_panel_ != nullptr) {
            RECT rc = panel_splitter_->GetPos();
            rc.left -= 3;
            rc.right += 3;
            if (PtInRect(&rc, pt)) {
                // 开始拖拽分割线：记录起点和初始宽度。
                ResetListDragState();
                splitter_dragging_ = true;
                splitter_drag_start_x_ = x;
                splitter_start_width_ = group_panel_->GetFixedWidth();
                splitter_pending_width_ = splitter_start_width_;
                splitter_last_update_tick_ = ::GetTickCount();
                SetCapture(m_hWnd);
                bHandled = TRUE;
                return 0;
            }
        }
    }

    if (uMsg == WM_MOUSEMOVE && list_drag_prepared_ && !list_dragging_) {
        const int x = static_cast<short>(LOWORD(lParam));
        const int y = static_cast<short>(HIWORD(lParam));
        const POINT pt = normalize_point(x, y);
        const int dx = std::abs(pt.x - list_drag_down_point_.x);
        const int dy = std::abs(pt.y - list_drag_down_point_.y);
        if ((wParam & MK_LBUTTON) == 0) {
            DebugLog("cancel prepared: lbutton released");
            ReleaseCapture();
            ResetListDragState();
            bHandled = TRUE;
            return 0;
        }
        if (dx >= kListDragStartThresholdPx || dy >= kListDragStartThresholdPx) {
            list_dragging_ = true;
            DebugLog("start drag from=" + std::to_string(list_drag_from_index_));
        }
    }

    if (uMsg == WM_MOUSEMOVE && list_dragging_) {
        const POINT pt = normalize_point(static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)));
        CListUI* target_list = nullptr;
        if (drag_list_kind_ == DragListKind::Groups) {
            target_list = groups_list_;
        } else if (drag_list_kind_ == DragListKind::Items) {
            target_list = items_list_;
        }

        const int hover_index = HitTestListIndex(target_list, pt);
        if (hover_index >= 0 && hover_index != list_drag_hover_index_) {
            list_drag_hover_index_ = hover_index;
            if (target_list != nullptr) {
                target_list->SelectItem(hover_index, false);
            }
            DebugLog("hover -> " + std::to_string(hover_index));
        }
        bHandled = TRUE;
        return 0;
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

        if (splitter_pending_width_ != next_width) {
            splitter_pending_width_ = next_width;
        }

        // 实时预览：拖拽过程中直接更新左侧宽度，提供连续反馈。
        // 使用轻量节流避免在高频鼠标消息下触发过多布局重算导致卡顿。
        const DWORD now = ::GetTickCount();
        const bool time_ready = (now - splitter_last_update_tick_) >= 12;
        const int current_width = group_panel_->GetFixedWidth();
        const bool delta_large = std::abs(current_width - splitter_pending_width_) >= 3;
        if ((time_ready || delta_large) && current_width != splitter_pending_width_) {
            group_panel_->SetFixedWidth(splitter_pending_width_);
            m_pm.NeedUpdate();
            splitter_last_update_tick_ = now;
        }
        bHandled = TRUE;
        return 0;
    }

    if (uMsg == WM_LBUTTONUP && splitter_dragging_) {
        // 结束拖拽：落最终值并标记持久化。
        splitter_dragging_ = false;
        ReleaseCapture();
        if (group_panel_ != nullptr && splitter_pending_width_ >= 0 && group_panel_->GetFixedWidth() != splitter_pending_width_) {
            group_panel_->SetFixedWidth(splitter_pending_width_);
            m_pm.NeedUpdate();
        }
        MarkUiStateDirty();
        bHandled = TRUE;
        return 0;
    }

    if (uMsg == WM_LBUTTONUP && list_dragging_) {
        ReleaseCapture();
        CommitListDragReorder();
        ResetListDragState();
        bHandled = TRUE;
        return 0;
    }

    if (uMsg == WM_LBUTTONUP && list_drag_prepared_) {
        ReleaseCapture();
        DebugLog("prepared only, no drag");
        ResetListDragState();
    }

    if (uMsg == WM_CAPTURECHANGED && splitter_dragging_) {
        splitter_dragging_ = false;
        if (group_panel_ != nullptr && splitter_pending_width_ >= 0 && group_panel_->GetFixedWidth() != splitter_pending_width_) {
            group_panel_->SetFixedWidth(splitter_pending_width_);
            m_pm.NeedUpdate();
        }
        MarkUiStateDirty();
        bHandled = TRUE;
        return 0;
    }

    if (uMsg == WM_CAPTURECHANGED && (list_dragging_ || list_drag_prepared_)) {
        // 列表拖拽依赖轮询定时器推进，捕获变化不立即打断。
        // 某些控件会在按下后短暂切换 capture，若此处直接重置会导致“只能 down 无法拖拽”。
        const bool left_down = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        if (!left_down) {
            if (list_dragging_) {
                CommitListDragReorder();
            }
            DebugLog("capture changed -> finalize drag state");
            ResetListDragState();
            bHandled = TRUE;
            return 0;
        }
        DebugLog("capture changed while holding left button, keep dragging");
    }

    // 鼠标拖动窗口边框结束后，统一触发一次状态持久化。
    if (uMsg == WM_EXITSIZEMOVE) {
        MarkUiStateDirty();
    }

    // 兜底：窗口尺寸变化（含程序内触发）也记录到配置。
    if (uMsg == WM_SIZE) {
        if (wParam != SIZE_MINIMIZED) {
            MarkUiStateDirty();
        }
    }

    if (uMsg == WM_TIMER && wParam == kUiStateSaveTimerId) {
        ::KillTimer(m_hWnd, kUiStateSaveTimerId);
        ui_state_timer_active_ = false;
        FlushUiStateIfDirty();
        bHandled = TRUE;
        return 0;
    }

    if (uMsg == WM_TIMER && wParam == kListDragPollTimerId) {
        if (!(list_drag_prepared_ || list_dragging_)) {
            ::KillTimer(m_hWnd, kListDragPollTimerId);
            list_drag_polling_ = false;
            bHandled = TRUE;
            return 0;
        }

        POINT pt_screen{};
        ::GetCursorPos(&pt_screen);
        POINT pt = pt_screen;
        ::ScreenToClient(m_hWnd, &pt);

        const bool left_down = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        if (!left_down) {
            if (list_dragging_) {
                CommitListDragReorder();
            }
            ReleaseCapture();
            DebugLog("poll release");
            ResetListDragState();
            bHandled = TRUE;
            return 0;
        }

        if (list_drag_prepared_ && !list_dragging_) {
            const int dx = std::abs(pt.x - list_drag_down_point_.x);
            const int dy = std::abs(pt.y - list_drag_down_point_.y);
            if (dx >= kListDragStartThresholdPx || dy >= kListDragStartThresholdPx) {
                list_dragging_ = true;
                DebugLog("start drag(from poll) from=" + std::to_string(list_drag_from_index_));
            }
        }

        if (list_dragging_) {
            CListUI* target_list = nullptr;
            if (drag_list_kind_ == DragListKind::Groups) {
                target_list = groups_list_;
            } else if (drag_list_kind_ == DragListKind::Items) {
                target_list = items_list_;
            }

            const int hover_index = HitTestListIndex(target_list, pt);
            if (hover_index >= 0 && hover_index != list_drag_hover_index_) {
                list_drag_hover_index_ = hover_index;
                if (target_list != nullptr) {
                    target_list->SelectItem(hover_index, false);
                }
                DebugLog("hover(from poll) -> " + std::to_string(hover_index));
            }
        }

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
        if (group_dialog_ != nullptr && group_dialog_->IsVisible()) {
            if (wParam == VK_RETURN) {
                ConfirmGroupDialog();
                bHandled = TRUE;
                return 0;
            }
            if (wParam == VK_ESCAPE) {
                CloseGroupDialog();
                bHandled = TRUE;
                return 0;
            }
        }
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
    if (splitter_dragging_) {
        splitter_dragging_ = false;
        ReleaseCapture();
    }
    if (ui_state_timer_active_) {
        ::KillTimer(m_hWnd, kUiStateSaveTimerId);
        ui_state_timer_active_ = false;
    }
    // 关闭窗口前强制落盘，确保最新窗口大小/位置被保存。
    SaveUiState();
    PostQuitMessage(0);
    bHandled = FALSE;
    return 0;
}
