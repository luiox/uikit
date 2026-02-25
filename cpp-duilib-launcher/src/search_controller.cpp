#include "search_controller.h"

#include "app_window.h"

SearchController::SearchController(AppWindow& owner)
    : owner_(owner) {}

bool SearchController::IsSearchMode() const {
    return owner_.search_mode_;
}

void SearchController::UpdateSearchUi() {
    if (owner_.search_bar_ != nullptr) {
        owner_.search_bar_->SetVisible(owner_.search_mode_);
        owner_.search_bar_->SetFixedHeight(owner_.search_mode_ ? 34 : 0);
    }
    if (owner_.group_panel_ != nullptr) {
        owner_.group_panel_->SetVisible(!owner_.search_mode_);
    }
    if (owner_.panel_splitter_ != nullptr) {
        owner_.panel_splitter_->SetVisible(!owner_.search_mode_);
    }
    if (owner_.search_input_ != nullptr) {
        owner_.search_input_->SetVisible(owner_.search_mode_);
    }
    owner_.m_pm.NeedUpdate();
    if (owner_.search_mode_ && owner_.search_input_ != nullptr) {
        owner_.search_input_->SetFocus();
        const int text_len = owner_.search_input_->GetText().GetLength();
        owner_.search_input_->SetSel(text_len, text_len);
    }
}

void SearchController::ToggleSearchMode() {
    owner_.search_mode_ = !owner_.search_mode_;
    if (!owner_.search_mode_ && owner_.search_input_ != nullptr) {
        owner_.search_input_->SetText(_T(""));
    }
    UpdateSearchUi();
    owner_.RenderItems();
    owner_.status_.Info(owner_.search_mode_ ? "search mode on" : "search mode off");
}

void SearchController::HandleInputChanged() {
    if (owner_.search_mode_) {
        owner_.RenderItems();
    }
}
