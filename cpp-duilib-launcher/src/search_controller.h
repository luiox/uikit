#pragma once

class AppWindow;

class SearchController {
public:
    explicit SearchController(AppWindow& owner);

    bool IsSearchMode() const;
    void UpdateSearchUi();
    void ToggleSearchMode();
    void HandleInputChanged();

private:
    AppWindow& owner_;
};
