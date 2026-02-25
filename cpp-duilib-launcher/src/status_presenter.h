#pragma once

#include <UIlib.h>

#include <string>

class StatusPresenter {
public:
    void Bind(DuiLib::CLabelUI* status_label);
    void Info(const std::string& text);
    void Warn(const std::string& text);
    void Error(const std::string& text);

private:
    DuiLib::CLabelUI* status_label_ = nullptr;
};
