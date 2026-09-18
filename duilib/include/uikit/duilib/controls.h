#pragma once

// 统一风格常用控件（L3）。全部从 ActiveTheme() 取色，切主题 = SetActiveTheme()
// + ApplyThemeToTree()，控件无需重建。
//
// fork 缺口说明（准入清单项）：此 fork 的 CButtonUI 状态色只能走图片
// （normalbkcolor 等属性不存在，静默忽略导致无底色），CCheckBoxUI 勾选态
// 依赖图片资源、无资源不可见。因此按钮族与勾选族在 PaintStatusImage 里
// 按主题色自绘——补的是"fork 状态色缺口"，不重包 duilib 原生能力。

#include <UIlib.h>

#include <cstddef>
#include <vector>

#include "uikit/duilib/compat.h"

namespace uikit::duilib {

// 主题感知接口：ApplyThemeToTree 按此识别 uikit 控件并在切换主题时通知重涂。
// uikit 自绘控件（按钮/勾选族）绘制时直读 ActiveTheme()，OnThemeChanged 默认
// 无需做事；构造期上色的复合控件（列表/顶栏/输入框）在重载里重新上色。
class ThemeableControl {
public:
    virtual ~ThemeableControl() = default;
    virtual void OnThemeChanged() {}
};

// —— 文本 ————————————————————————————————————————————————

// 标签：role 决定正文/弱化色，每帧从 ActiveTheme() 解析，切主题自动跟上。
class LabelUI : public DuiLib::CLabelUI, public ThemeableControl {
public:
    enum class Role { Default, Secondary };
    LPCTSTR GetClass() const override { return _T("UIKitLabel"); }
    void SetRole(Role role) { role_ = role; }

protected:
    void PaintText(DuiLib::UIRender* pRender) override;

private:
    Role role_ = Role::Default;
};

// 主题化面板容器：bkcolor 恒等于主题 panel，作窗口根/分区底。不设 bkcolor 的
// 透明容器直接叠在它上面即可，无需逐个主题化。
class PanelUI : public DuiLib::CVerticalLayoutUI, public ThemeableControl {
public:
    LPCTSTR GetClass() const override { return _T("UIKitPanel"); }
    void OnThemeChanged() override;
};

// —— 按钮族 ——————————————————————————————————————————————

// 状态色自绘按钮。默认透明底（工具条/顶栏用法），StylePrimary/StyleSecondary
// 一键套用主题的强调/次级样式。样式只存"角色"，颜色每帧从 ActiveTheme()
// 解析——切主题后无需重建控件。
class ButtonUI : public DuiLib::CButtonUI, public ThemeableControl {
public:
    LPCTSTR GetClass() const override { return _T("UIKitButton"); }
    // 显式状态色（mlaunch 迁移兼容）；设置后不再跟随主题预设。
    void SetStateColors(Color normal, Color hot, Color pushed);
    // 粘滞按下态（如搜索模式下的放大镜）：激活期间恒显示 pushed 色。
    void SetActive(bool active);
    // 主题预设：primary = accent 底 + on_accent 文字；secondary = panel 底。
    void StylePrimary();
    void StyleSecondary();

protected:
    void PaintStatusImage(DuiLib::UIRender* pRender) override;
    void PaintText(DuiLib::UIRender* pRender) override;
    void ResolveStateColors(Color& normal, Color& hot, Color& pushed) const;

private:
    enum class Style { Transparent, Primary, Secondary, Custom };
    Style style_ = Style::Transparent;
    Color custom_normal_ = color::kTransparent;
    Color custom_hot_ = color::kTransparent;
    Color custom_pushed_ = color::kTransparent;
    bool active_ = false;
};

// 顶栏图标按钮：等大正方形，常态透明，悬停才显灰底。
class IconButtonUI : public ButtonUI {
public:
    IconButtonUI();
    LPCTSTR GetClass() const override { return _T("UIKitIconButton"); }
    void SetSvgImage(const DuiLib::CDuiString& image_attr);
};

// 便捷构造：主体系文本按钮（secondary 样式，高 control_height，无边框）。
DuiLib::CButtonUI* MakeTextButton(LPCTSTR name, LPCTSTR text, int width = 0);

// —— 勾选/单选族 ————————————————————————————————————————

// 自绘复选框：主题 accent 勾选底 + 圆角盒，DPI 缩放，点击自切换并发 CLICK。
class CheckBoxUI : public ButtonUI {
public:
    CheckBoxUI();
    LPCTSTR GetClass() const override { return _T("UIKitCheckBox"); }
    void SetChecked(bool checked);
    bool IsChecked() const { return checked_; }

protected:
    bool Activate() override;
    void PaintStatusImage(DuiLib::UIRender* pRender) override;

private:
    bool checked_ = false;
};

// 自绘单选钮：checked = accent 外环 + accent 圆点；unchecked = 中性外环。
class RadioButtonUI : public ButtonUI {
public:
    RadioButtonUI();
    LPCTSTR GetClass() const override { return _T("UIKitRadioButton"); }
    void SetChecked(bool checked);
    bool IsChecked() const { return checked_; }
    // 同组互斥由宿主处理（duilib 无内建 radio 组语义）；本方法只切视觉。
    // 便捷：传入同组兄弟列表，Set 组内选中并互斥。
    void SetGroup(DuiLib::CControlUI* const* siblings, size_t count);

protected:
    bool Activate() override;
    void PaintStatusImage(DuiLib::UIRender* pRender) override;

private:
    void UncheckSiblings();
    bool checked_ = false;
    std::vector<DuiLib::CControlUI*> siblings_;
};

// —— 列表族 ——————————————————————————————————————————————

// 分组列表：panel 底，选中条 panel_selected。构造期上色，主题切换时由
// OnThemeChanged 重涂。
class GroupListUI : public DuiLib::CListUI, public ThemeableControl {
public:
    GroupListUI();
    LPCTSTR GetClass() const override { return _T("UIKitGroupList"); }
    void OnThemeChanged() override;
};

// 条目列表：surface 底，选中条 surface_selected。
class ItemListUI : public DuiLib::CListUI, public ThemeableControl {
public:
    ItemListUI();
    LPCTSTR GetClass() const override { return _T("UIKitItemList"); }
    void OnThemeChanged() override;
};

// 分组行：选中条带 1px 描边（列表级 item 背景只能填色，描边由行末补画）。
class GroupRowUI : public DuiLib::CListContainerElementUI, public ThemeableControl {
public:
    LPCTSTR GetClass() const override { return _T("UIKitGroupRow"); }

protected:
    bool DoPaint(DuiLib::UIRender* pRender, const DuiLib::CDuiRect& rcPaint,
                 DuiLib::CControlUI* pStopControl) override;
};

// 极简滚动条：无箭头、固定宽度 metrics.scrollbar_width、纯色轨道；
// thumb 走九宫格图（消费方可用 micon 生成），缺省只显示轨道。
void ApplyFlatScrollbar(DuiLib::CListUI* list, Color track_color,
                        const DuiLib::CDuiString& thumb_attr = DuiLib::CDuiString());

// —— 输入/顶栏 ————————————————————————————————————————————

// 搜索输入框：surface 底无边框，高度/内边距走 metrics。
class SearchBoxUI : public DuiLib::CEditUI, public ThemeableControl {
public:
    SearchBoxUI();
    LPCTSTR GetClass() const override { return _T("UIKitSearchBox"); }
    void OnThemeChanged() override;
};

// 无边框窗口标题栏：panel 底、metrics.titlebar_height 高、childvalign center。
// 拖拽/双击最大化命中由 fork 的 caption 语义承接（配合 frameless.h 使用）。
class TitleBarUI : public DuiLib::CHorizontalLayoutUI, public ThemeableControl {
public:
    TitleBarUI();
    LPCTSTR GetClass() const override { return _T("UIKitTitleBar"); }
    void OnThemeChanged() override;
};

}  // namespace uikit::duilib
