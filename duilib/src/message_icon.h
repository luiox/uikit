#pragma once

// 级别图标（L3 内部共用件，不安装）：淡色圆底 + 主题状态色字形。
// 纯绘制无图片资产；micon SVG 语义映射接通后可整体替换为矢量图标。
// 消息框（messagebox.cpp）与悬浮通知（toast.cpp）共用。

#include <UIlib.h>

#include "uikit/duilib/controls.h"    // ThemeableControl/scale 所在的包装层
#include "uikit/duilib/messagebox.h"  // MessageLevel

namespace uikit::detail {

// 级别图标几何常量（96 基准）：外圈直径与字形字号。
constexpr int kLevelIconSize = 36;
constexpr int kLevelGlyphFontId = 10;  // AddFont 显式 id，避开主题字体的 1..N 段

class MessageIconUI : public DuiLib::CControlUI {
public:
    explicit MessageIconUI(MessageLevel level) : level_(level) {
        SetFixedWidth(kLevelIconSize);
        SetFixedHeight(kLevelIconSize);
    }

    bool Paint(DuiLib::UIRender* pRender, const DuiLib::CDuiRect& rcPaint,
               DuiLib::CControlUI* pStopControl) override {
        const ResolvedTheme& t = ActiveTheme();
        Color main = t.color.accent;
        LPCTSTR glyph = _T("i");
        switch (level_) {
            case MessageLevel::Success: main = t.color.success; glyph = _T("✓"); break;
            case MessageLevel::Warning: main = t.color.warning; glyph = _T("!"); break;
            case MessageLevel::Error:   main = t.color.danger;  glyph = _T("✕"); break;
            case MessageLevel::Question: main = t.color.accent; glyph = _T("?"); break;
            case MessageLevel::Info: break;
        }
        pRender->FillEllipse(m_rcItem, color::mix(main, t.color.surface, 0.85));
        DuiLib::CDuiRect rc = m_rcItem;
        pRender->DrawText(rc, glyph, main, kLevelGlyphFontId,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return DuiLib::CControlUI::Paint(pRender, rcPaint, pStopControl);
    }

private:
    MessageLevel level_;
};

}  // namespace uikit::detail
