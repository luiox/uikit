#include "list_controller.h"

#include <algorithm>

#include "app_window.h"
#include "file_icon_control.h"
#include "utils/string_util.h"

using namespace DuiLib;

ListController::ListController(AppWindow& owner)
    : owner_(owner) {}

void ListController::RenderGroups() {
    if (!owner_.groups_list_) {
        return;
    }
    owner_.groups_list_->RemoveAll();
    owner_.group_ids_.clear();

    std::vector<const backend::Group*> groups;
    groups.reserve(owner_.backend_.Data().groups.size());
    for (const auto& group : owner_.backend_.Data().groups) {
        groups.push_back(&group);
    }
    std::sort(groups.begin(), groups.end(), [](const backend::Group* lhs, const backend::Group* rhs) {
        return lhs->order < rhs->order;
    });

    for (const auto* group : groups) {
        auto* row = new CListContainerElementUI();
        row->SetFixedHeight(34);

        auto* name = new CLabelUI();
        name->SetText(launcher::util::Utf8ToWide(group->name).c_str());
        name->SetAttribute(_T("padding"), _T("8,0,0,0"));
        name->SetTextColor(0xFF5A5A5A);
        name->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        row->Add(name);

        owner_.groups_list_->Add(row);
        owner_.group_ids_.push_back(group->id);
    }
}

void ListController::RenderItems() {
    if (!owner_.items_list_) {
        return;
    }

    owner_.items_list_->RemoveAll();
    owner_.item_ids_.clear();
    owner_.item_group_ids_.clear();
    owner_.selected_item_id_.clear();
    owner_.selected_item_group_id_.clear();

    if (owner_.search_mode_) {
        std::string keyword;
        if (owner_.search_input_ != nullptr) {
            keyword = launcher::util::WideToUtf8(owner_.search_input_->GetText().GetData());
        }

        for (const auto& group : owner_.backend_.Data().groups) {
            for (const auto& item : group.items) {
                if (item.item_type == "separator") {
                    continue;
                }
                if (!keyword.empty() && !owner_.ContainsCaseInsensitive(item.name, keyword)) {
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
                icon->SetIconPath(launcher::util::Utf8ToWide(owner_.icon_manager_.ParseItemIconSource(item)));
                row->Add(icon);

                auto* name = new CLabelUI();
                name->SetText(launcher::util::Utf8ToWide(item.name + "  [" + group.name + "]").c_str());
                name->SetTextColor(0xFF5A5A5A);
                name->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                row->Add(name);

                owner_.items_list_->Add(row);
                owner_.item_ids_.push_back(item.id);
                owner_.item_group_ids_.push_back(group.id);
            }
        }
        return;
    }

    const backend::Group* group = owner_.FindActiveGroup();
    if (!group) {
        return;
    }

    for (const auto& item : group->items) {
        if (item.item_type == "separator") {
            auto* row = new CListContainerElementUI();
            row->SetFixedHeight(34);

            auto* name = new CLabelUI();
            name->SetText(launcher::util::Utf8ToWide(item.name).c_str());
            name->SetAttribute(_T("padding"), _T("10,0,0,0"));
            name->SetTextColor(0xFF909090);
            name->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            row->Add(name);

            owner_.items_list_->Add(row);
            owner_.item_ids_.push_back(item.id);
            owner_.item_group_ids_.push_back(group->id);
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
        icon->SetIconPath(launcher::util::Utf8ToWide(owner_.icon_manager_.ParseItemIconSource(item)));
        row->Add(icon);

        auto* name = new CLabelUI();
        name->SetText(launcher::util::Utf8ToWide(item.name).c_str());
        name->SetTextColor(0xFF5A5A5A);
        name->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        row->Add(name);

        owner_.items_list_->Add(row);
        owner_.item_ids_.push_back(item.id);
        owner_.item_group_ids_.push_back(group->id);
    }
}

void ListController::SelectGroupByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(owner_.group_ids_.size())) {
        return;
    }
    owner_.active_group_id_ = owner_.group_ids_[index];
    if (owner_.groups_list_) {
        owner_.groups_list_->SelectItem(index, false);
    }
    RenderItems();
}

bool ListController::SelectListRowFromPoint(CListUI* list, const std::vector<std::string>& ids, const POINT& client_point, std::string* selected_id) {
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
