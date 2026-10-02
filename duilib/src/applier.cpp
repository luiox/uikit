#include "uikit/duilib/applier.h"

#include "uikit/duilib/controls.h"

namespace uikit::duilib {

using DuiLib::CDuiString;

void ApplyThemeToTree(DuiLib::CControlUI* root) {
    if (root == nullptr) {
        return;
    }
    if (auto* themeable = dynamic_cast<ThemeableControl*>(root)) {
        themeable->OnThemeChanged();
    } else if (auto* label = dynamic_cast<DuiLib::CLabelUI*>(root)) {
        label->SetTextColor(ActiveTheme().color.text);
    }
    root->Invalidate();
    // 该 fork 的子节点接口在 CContainerUI 上，非叶控件统一走这里。
    if (auto* container = dynamic_cast<DuiLib::CContainerUI*>(root)) {
        for (int i = 0; i < container->GetCount(); ++i) {
            ApplyThemeToTree(container->GetItemAt(i));
        }
    }
}

namespace {

// tokens 里 family 存 UTF-8 窄串；duilib 在 UNICODE 构建下要宽字符。
DuiLib::CDuiString to_wide(const std::string& utf8) {
    if (utf8.empty()) {
        return DuiLib::CDuiString();
    }
    const int need = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(),
                                         static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(need), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), &wide[0], need);
    return DuiLib::CDuiString(wide.c_str());
}

}  // namespace

int InstallFonts(DuiLib::CPaintManagerUI& pm, const ResolvedTheme& theme, int size_px,
                 int* out_extra_count) {
    const FontToken* default_font = nullptr;
    for (const auto& f : theme.fonts) {
        if (f.role == "default") {
            default_font = &f;
            break;
        }
    }
    const CDuiString family =
        to_wide(default_font != nullptr ? default_font->family : "Microsoft YaHei UI");
    pm.SetDefaultFont(family.GetData(), size_px, FALSE, FALSE, FALSE, FALSE);

    // 非默认角色按 fonts 顺序注册，索引从 1 起；weight >= 600 视为粗体。
    int extra = 0;
    for (const auto& f : theme.fonts) {
        if (f.role == "default") {
            continue;
        }
        ++extra;
        pm.AddFont(extra, to_wide(f.family).GetData(), size_px, f.weight >= 600 ? TRUE : FALSE,
                   FALSE, FALSE);
    }
    if (out_extra_count != nullptr) {
        *out_extra_count = extra;
    }
    return 0;
}

}  // namespace uikit::duilib
