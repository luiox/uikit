#pragma once

// L3 薄适配层：Color ↔ duilib 属性/DPI 的换算。只放无状态小工具，
// 控件与应用逻辑请走 controls.h / applier.h / frameless.h。

#include <UIlib.h>

#include "uikit/theme/color.h"
#include "uikit/theme/theme.h"

namespace uikit::duilib {

// Color → duilib 属性串（"0xAARRGGBB"，SetAttribute 系接口使用）。
inline DuiLib::CDuiString attr(Color c) {
    return DuiLib::CDuiString(color::to_string(c).c_str());
}

// 96 基准逻辑像素 → 当前窗口 DPI 像素。manager 为空（罕见）按原值返回。
inline int scale(DuiLib::CPaintManagerUI* manager, int logical_px) {
    auto* dpi = (manager != nullptr) ? manager->GetDPIObj() : nullptr;
    return (dpi != nullptr) ? dpi->ScaleInt(logical_px) : logical_px;
}

}  // namespace uikit::duilib
