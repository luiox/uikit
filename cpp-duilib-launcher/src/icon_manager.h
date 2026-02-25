#pragma once

#include <UIlib.h>

#include <filesystem>
#include <string>

#include "backend.h"
#include "icons.h"

class IconManager {
public:
    explicit IconManager(std::filesystem::path base_dir);

    std::string ParseItemIconSource(const backend::LaunchItem& item) const;
    iconlib::Icon ResolveTopBarIcon(iconlib::Icon preferred, iconlib::Icon fallback) const;
    DuiLib::CDuiString MakeSvgImageAttr(iconlib::Icon icon, int draw_px = 16, int box_px = 26) const;

private:
    static void ReplaceAllInPlace(std::string* text, const std::string& from, const std::string& to);
    static std::string ApplyIconThemeColor(std::string svg_text);
    static bool WriteThemedSvg(const std::filesystem::path& out_path, const std::string& raw_svg);
    std::filesystem::path BuildThemedIconPath(const std::filesystem::path& source_or_name) const;
    std::filesystem::path GetEmbeddedIconPath(iconlib::Icon icon) const;
    std::filesystem::path GetDynamicIconPath(iconlib::Icon icon) const;
    std::filesystem::path ResolveIconPath(iconlib::Icon icon) const;

private:
    std::filesystem::path cache_dir_;
};
