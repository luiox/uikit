#include "status_presenter.h"

#include "constants.h"
#include "utils/string_util.h"

void StatusPresenter::Bind(DuiLib::CLabelUI* status_label) {
    status_label_ = status_label;
}

void StatusPresenter::Info(const std::string& text) {
    if (!status_label_) {
        return;
    }
    status_label_->SetText(launcher::util::Utf8ToWide(text).c_str());
    status_label_->SetTextColor(launcher::constants::color::kStatusInfo);
}

void StatusPresenter::Warn(const std::string& text) {
    if (!status_label_) {
        return;
    }
    status_label_->SetText(launcher::util::Utf8ToWide(text).c_str());
    status_label_->SetTextColor(launcher::constants::color::kStatusWarn);
}

void StatusPresenter::Error(const std::string& text) {
    if (!status_label_) {
        return;
    }
    status_label_->SetText(launcher::util::Utf8ToWide(text).c_str());
    status_label_->SetTextColor(launcher::constants::color::kStatusError);
}
