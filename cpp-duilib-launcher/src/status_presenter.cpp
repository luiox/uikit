#include "status_presenter.h"

#include <Windows.h>

void StatusPresenter::Bind(DuiLib::CLabelUI* status_label) {
    status_label_ = status_label;
}

std::wstring StatusPresenter::Utf8ToWide(const std::string& text) {
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

void StatusPresenter::Info(const std::string& text) {
    if (!status_label_) {
        return;
    }
    status_label_->SetText(Utf8ToWide(text).c_str());
    status_label_->SetTextColor(0xFF445A72);
}

void StatusPresenter::Warn(const std::string& text) {
    if (!status_label_) {
        return;
    }
    status_label_->SetText(Utf8ToWide(text).c_str());
    status_label_->SetTextColor(0xFF9A6700);
}

void StatusPresenter::Error(const std::string& text) {
    if (!status_label_) {
        return;
    }
    status_label_->SetText(Utf8ToWide(text).c_str());
    status_label_->SetTextColor(0xFFB00020);
}
