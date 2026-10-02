// uikit demo：主题（深/浅实时切换）+ 统一控件目检窗口。
// 纯代码构建（与 mlaunch 同款做法），无 XML 资产依赖。

#include <UIlib.h>

#include <tchar.h>

#include "uikit/duilib/applier.h"
#include "uikit/duilib/combobox.h"
#include "uikit/duilib/controls.h"
#include "uikit/duilib/datepicker.h"
#include "uikit/duilib/edit.h"
#include "uikit/duilib/frame_window.h"
#include "uikit/duilib/frameless.h"
#include "uikit/duilib/menu.h"
#include "uikit/duilib/messagebox.h"
#include "uikit/duilib/progress.h"
#include "uikit/duilib/slider.h"
#include "uikit/duilib/spinbox.h"
#include "uikit/duilib/switch.h"
#include "uikit/duilib/tabcontrol.h"
#include "uikit/duilib/toast.h"
#include "uikit/duilib/tooltip.h"

using namespace DuiLib;
using namespace uikit;
using namespace uikit::duilib;
using namespace uikit::duilib;

namespace {

// 演示用 /msgbox 自动开框定时器基址（250ms 一次性触发，见 OnCreate/HandleMessage）
constexpr UINT_PTR kMsgboxTimerId = 100;
// 演示用 /toast 自动弹通知定时器 id
constexpr UINT_PTR kToastTimerId = 110;
// 演示用 /tip /menu /date 弹层验证定时器基址
constexpr UINT_PTR kVerifyTimerId = 120;

DuiLib::CControlUI* MakeFixedSpacer(int height) {
    auto* spacer = new DuiLib::CControlUI();
    spacer->SetFixedHeight(height);
    return spacer;
}

// FrameWindow 预制件演示窗（/frame、/frame_max 打开）：主题化标题栏 +
// 最小化/最大化/关闭钮，内容区 InitContent 装配。
class FrameDemoWindow : public FrameWindow {
public:
    LPCTSTR GetWindowClassName() const override { return _T("UIKitFrameDemo"); }

protected:
    void InitContent(DuiLib::CContainerUI* content) override {
        content->SetAttribute(_T("inset"), _T("16,16,16,16"));
        auto* label = new LabelUI();
        label->SetText(
            _T("FrameWindow 预制件：主题化标题栏（最小化/最大化/关闭）。拖拽、"
               "双击最大化、边缘缩放沿 FramelessWindow；最大化时 NC 框厚已修正。"));
        label->SetTextStyle(DT_LEFT | DT_WORDBREAK);
        content->Add(label);
    }
    void OnFinalMessage(HWND) override { delete this; }
};

class DemoWindow : public FramelessWindow {
public:
    LPCTSTR GetWindowClassName() const override { return _T("UIKitDemo"); }

    DuiLib::CControlUI* BuildRootUi() override {
        const ResolvedTheme& t = ActiveTheme();

        auto* root = new PanelUI();
        root->SetName(_T("root"));
        root->SetAttribute(_T("bordersize"), _T("0"));
        root->OnThemeChanged();

        // —— 顶栏 ——
        auto* top_bar = new TitleBarUI();
        auto* title = new LabelUI();
        title->SetText(_T("uikit · 主题与控件"));
        title->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        top_bar->Add(title);
        top_bar->Add(new DuiLib::CControlUI());
        top_bar->Add(MakeTextButton(_T("toggle_theme"), _T("切换主题"), 72));
        root->Add(top_bar);

        // —— 主体：左侧控件面板 + 右侧列表 ——
        auto* body = new DuiLib::CHorizontalLayoutUI();
        body->SetAttribute(_T("inset"), _T("16,12,16,16"));
        body->SetAttribute(_T("childpadding"), _T("12"));

        auto* panel = new DuiLib::CVerticalLayoutUI();
        panel->SetFixedWidth(280);
        panel->SetAttribute(_T("childpadding"), _T("6"));

        auto* settings_label = new LabelUI();
        settings_label->SetText(_T("设置"));
        settings_label->SetRole(LabelUI::Role::Secondary);
        settings_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        panel->Add(settings_label);

        auto* check_auto = new CheckBoxUI();
        check_auto->SetName(_T("check_auto"));
        check_auto->SetText(_T("开机自动启动"));
        check_auto->SetChecked(true);
        panel->Add(check_auto);

        auto* check_tray = new CheckBoxUI();
        check_tray->SetName(_T("check_tray"));
        check_tray->SetText(_T("关闭时最小化到托盘"));
        panel->Add(check_tray);

        auto* check_disabled = new CheckBoxUI();
        check_disabled->SetName(_T("check_disabled"));
        check_disabled->SetText(_T("禁用态复选框"));
        check_disabled->SetEnabled(false);
        panel->Add(check_disabled);

        panel->Add(MakeFixedSpacer(6));

        auto* theme_label = new LabelUI();
        theme_label->SetText(_T("外观"));
        theme_label->SetRole(LabelUI::Role::Secondary);
        theme_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        panel->Add(theme_label);

        radio_light_ = new RadioButtonUI();
        radio_light_->SetName(_T("radio_light"));
        radio_light_->SetText(_T("浅色主题"));
        radio_light_->SetChecked(ActiveTheme().appearance == color::Appearance::Light);
        radio_dark_ = new RadioButtonUI();
        radio_dark_->SetName(_T("radio_dark"));
        radio_dark_->SetText(_T("深色主题"));
        radio_dark_->SetChecked(ActiveTheme().appearance == color::Appearance::Dark);
        DuiLib::CControlUI* group[] = {radio_light_, radio_dark_};
        radio_light_->SetGroup(group, 2);
        radio_dark_->SetGroup(group, 2);
        panel->Add(radio_light_);
        panel->Add(radio_dark_);

        panel->Add(MakeFixedSpacer(6));

        auto* search = new SearchBoxUI();
        search->SetName(_T("search"));
        search->SetText(_T("搜索…"));
        panel->Add(search);

        panel->Add(MakeFixedSpacer(6));

        auto* input_label = new LabelUI();
        input_label->SetText(_T("输入"));
        input_label->SetRole(LabelUI::Role::Secondary);
        input_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        panel->Add(input_label);

        auto* edit = new Edit();
        edit->SetName(_T("edit"));
        edit->SetTipValue(_T("输入内容，回车确认…"));
        panel->Add(edit);

        auto* edit_pwd = new Edit();
        edit_pwd->SetName(_T("edit_pwd"));
        edit_pwd->SetPasswordMode(true);
        edit_pwd->SetTipValue(_T("密码"));
        panel->Add(edit_pwd);

        panel->Add(MakeFixedSpacer(6));

        auto* switch_label = new LabelUI();
        switch_label->SetText(_T("开关 / 滑杆 / 选择"));
        switch_label->SetRole(LabelUI::Role::Secondary);
        switch_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        panel->Add(switch_label);

        auto* notify_row = new DuiLib::CHorizontalLayoutUI();
        notify_row->SetFixedHeight(t.metrics.control_height);
        notify_row->SetAttribute(_T("childvalign"), _T("center"));
        auto* notify_label = new LabelUI();
        notify_label->SetText(_T("桌面通知"));
        notify_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        notify_row->Add(notify_label);
        notify_row->Add(new DuiLib::CControlUI());
        auto* notify_switch = new Switch();
        notify_switch->SetName(_T("notify_switch"));
        notify_switch->SetChecked(true);
        notify_row->Add(notify_switch);
        panel->Add(notify_row);

        auto* slider = new Slider();
        slider->SetName(_T("slider"));
        slider->SetValue(40);
        panel->Add(slider);

        auto* combo = new ComboBox();
        combo->SetName(_T("combo"));
        combo->AddOption(_T("自动（推荐）"));
        combo->AddOption(_T("1920 × 1080"));
        combo->AddOption(_T("1280 × 720"));
        combo->SelectItem(0, false, false);
        panel->Add(combo);

        auto* spin = new SpinBox();
        spin->SetName(_T("spin"));
        spin->SetValueRange(1, 72);
        spin->SetValue(12);
        panel->Add(spin);

        auto* date = new DatePicker();
        date->SetName(_T("date"));
        panel->Add(date);

        panel->Add(MakeFixedSpacer(6));

        auto* progress_label = new LabelUI();
        progress_label->SetText(_T("进度"));
        progress_label->SetRole(LabelUI::Role::Secondary);
        progress_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        panel->Add(progress_label);

        auto* progress = new ProgressBar();
        progress->SetName(_T("progress"));
        progress->SetValue(35);
        panel->Add(progress);

        panel->Add(MakeFixedSpacer(4));

        auto* progress_ops = new DuiLib::CHorizontalLayoutUI();
        progress_ops->SetFixedHeight(t.metrics.control_height);
        progress_ops->SetAttribute(_T("childpadding"), _T("8"));
        progress_ops->Add(MakeTextButton(_T("progress_dec"), _T("-10"), 48));
        progress_ops->Add(MakeTextButton(_T("progress_inc"), _T("+10"), 48));
        progress_ops->Add(new DuiLib::CControlUI());
        progress_ops->Add(MakeTextButton(_T("msgbox_info"), _T("消息框…"), 84));
        panel->Add(progress_ops);

        panel->Add(MakeFixedSpacer(10));

        auto* actions = new DuiLib::CHorizontalLayoutUI();
        actions->SetFixedHeight(t.metrics.control_height);
        actions->SetAttribute(_T("childpadding"), _T("8"));
        auto* save = new ButtonUI();
        save->SetName(_T("save"));
        save->SetText(_T("保存设置"));
        save->SetFixedWidth(96);
        save->StylePrimary();
        actions->Add(save);
        actions->Add(MakeTextButton(_T("cancel"), _T("取消"), 72));
        actions->Add(new DuiLib::CControlUI());
        auto* disabled_btn = MakeTextButton(_T("disabled_btn"), _T("禁用"));
        disabled_btn->SetEnabled(false);
        actions->Add(disabled_btn);
        panel->Add(actions);

        body->Add(panel);

        // —— 右侧：分组列表 + 条目列表（滚动条走极简样式）——
        auto* lists = new DuiLib::CVerticalLayoutUI();
        lists->SetAttribute(_T("childpadding"), _T("12"));

        auto* groups = new GroupListUI();
        groups->SetName(_T("groups"));
        const LPCTSTR group_names[] = {_T("启动项"), _T("系统工具"), _T("开发"), _T("娱乐")};
        for (LPCTSTR name : group_names) {
            auto* row = new GroupRowUI();
            row->SetFixedHeight(t.metrics.control_height);
            auto* label = new LabelUI();
            label->SetText(name);
            label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DuiLib::CDuiString inset;
            inset.Format(_T("%d,0,0,0"), t.metrics.control_hpad);
            label->SetAttribute(_T("inset"), inset.GetData());
            row->Add(label);
            groups->Add(row);
        }
        lists->Add(groups);

        auto* items = new ItemListUI();
        items->SetName(_T("items"));
        const LPCTSTR item_names[] = {_T("记事本 notepad"), _T("计算器 calc"),
                                      _T("画图 mspaint"),   _T("任务管理器 taskmgr"),
                                      _T("注册表 regedit")};
        for (LPCTSTR name : item_names) {
            auto* row = new DuiLib::CListContainerElementUI();
            row->SetFixedHeight(t.metrics.control_height);
            auto* label = new LabelUI();
            label->SetText(name);
            label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DuiLib::CDuiString inset;
            inset.Format(_T("%d,0,0,0"), t.metrics.control_hpad);
            label->SetAttribute(_T("inset"), inset.GetData());
            row->Add(label);
            items->Add(row);
        }
        lists->Add(items);
        body->Add(lists);

        root->Add(body);

        // —— 消息框：模态/非模态 × 归属/独立 HWND 两正交维演示 ——
        auto* msg_box_section = new DuiLib::CVerticalLayoutUI();
        msg_box_section->SetAttribute(_T("childpadding"), _T("6"));
        auto* msg_label = new LabelUI();
        msg_label->SetText(_T("消息框（模态 × 独立 HWND）"));
        msg_label->SetRole(LabelUI::Role::Secondary);
        msg_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        msg_box_section->Add(msg_label);

        auto* msg_row1 = new DuiLib::CHorizontalLayoutUI();
        msg_row1->SetFixedHeight(t.metrics.control_height);
        msg_row1->SetAttribute(_T("childpadding"), _T("8"));
        msg_row1->Add(MakeTextButton(_T("mb_modal_owned"), _T("模态·归属"), 88));
        msg_row1->Add(MakeTextButton(_T("mb_modal_free"), _T("模态·独立"), 88));
        msg_row1->Add(new DuiLib::CControlUI());
        msg_box_section->Add(msg_row1);

        auto* msg_row2 = new DuiLib::CHorizontalLayoutUI();
        msg_row2->SetFixedHeight(t.metrics.control_height);
        msg_row2->SetAttribute(_T("childpadding"), _T("8"));
        msg_row2->Add(MakeTextButton(_T("mb_modeless_owned"), _T("非模态·归属"), 88));
        msg_row2->Add(MakeTextButton(_T("mb_modeless_free"), _T("非模态·独立"), 88));
        msg_row2->Add(new DuiLib::CControlUI());
        msg_box_section->Add(msg_row2);

        auto* msg_row3 = new DuiLib::CHorizontalLayoutUI();
        msg_row3->SetFixedHeight(t.metrics.control_height);
        msg_row3->SetAttribute(_T("childpadding"), _T("8"));
        msg_row3->Add(MakeTextButton(_T("toast_info"), _T("悬浮通知"), 88));
        msg_row3->Add(MakeTextButton(_T("toast_warn"), _T("告警通知"), 88));
        msg_row3->Add(new DuiLib::CControlUI());
        msg_box_section->Add(msg_row3);
        lists->Add(msg_box_section);

        // —— 页签 + 弹层入口：TabControl / Tooltip / Menu ——
        auto* tab_section = new DuiLib::CVerticalLayoutUI();
        tab_section->SetAttribute(_T("childpadding"), _T("6"));
        auto* tab_label = new LabelUI();
        tab_label->SetText(_T("页签 / 弹层"));
        tab_label->SetRole(LabelUI::Role::Secondary);
        tab_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        tab_section->Add(tab_label);

        auto* tabs = new TabControl();
        tabs->SetName(_T("tabs"));
        auto* page1 = new PanelUI();
        page1->OnThemeChanged();
        page1->SetAttribute(_T("inset"), _T("12,10,12,10"));
        auto* page1_label = new LabelUI();
        page1_label->SetText(_T("页签一：常规设置内容区。"));
        page1_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        page1->Add(page1_label);
        auto* page2 = new PanelUI();
        page2->OnThemeChanged();
        page2->SetAttribute(_T("inset"), _T("12,10,12,10"));
        auto* page2_label = new LabelUI();
        page2_label->SetText(_T("页签二：高级选项内容区。"));
        page2_label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        page2->Add(page2_label);
        tabs->AddPage(page1, _T("常规"));
        tabs->AddPage(page2, _T("高级"));
        tabs->SetFixedHeight(120);
        tab_section->Add(tabs);

        auto* pop_row = new DuiLib::CHorizontalLayoutUI();
        pop_row->SetFixedHeight(t.metrics.control_height);
        pop_row->SetAttribute(_T("childpadding"), _T("8"));
        pop_row->Add(MakeTextButton(_T("tip_btn"), _T("悬浮提示"), 88));
        pop_row->Add(MakeTextButton(_T("menu_btn"), _T("上下文菜单"), 88));
        pop_row->Add(new DuiLib::CControlUI());
        tab_section->Add(pop_row);
        lists->Add(tab_section);

        // 极简滚动条（轨道色随主题语境：分组区 panel、条目区 surface）。
        groups_ = groups;
        items_ = items;
        ApplyFlatScrollbar(groups_, ActiveTheme().color.panel);
        ApplyFlatScrollbar(items_, ActiveTheme().color.surface);
        return root;
    }

    void Notify(DuiLib::TNotifyUI& msg) override {
        if (msg.sType != _T("click")) {
            return;
        }
        const DuiLib::CDuiString& name = msg.pSender->GetName();
        if (name == _T("close") || name == _T("cancel")) {
            Close();
            return;
        }
        if (name == _T("toggle_theme") || name == _T("radio_light") || name == _T("radio_dark")) {
            const bool want_dark =
                (name == _T("toggle_theme"))
                    ? ActiveTheme().appearance == color::Appearance::Light
                    : (name == _T("radio_dark"));
            if (ActiveTheme().appearance == color::Appearance::Light && want_dark) {
                SetActiveTheme(DarkTheme());
            } else if (ActiveTheme().appearance == color::Appearance::Dark && !want_dark) {
                SetActiveTheme(LightTheme());
            }
            if (name == _T("toggle_theme")) {
                radio_light_->SetChecked(!want_dark);
                radio_dark_->SetChecked(want_dark);
            }
            ApplyThemeToTree(paint_manager().GetRoot());
            // 滚动条轨道色是存储值，切主题后重套。
            ApplyFlatScrollbar(groups_, ActiveTheme().color.panel);
            ApplyFlatScrollbar(items_, ActiveTheme().color.surface);
            return;
        }
        if (name == _T("progress_dec") || name == _T("progress_inc")) {
            auto* bar = paint_manager().FindControl(_T("progress"));
            if (bar != nullptr) {
                int value = static_cast<DuiLib::CProgressUI*>(bar)->GetValue();
                value += (name == _T("progress_inc")) ? 10 : -10;
                if (value < 0) value = 0;
                if (value > 100) value = 100;
                static_cast<DuiLib::CProgressUI*>(bar)->SetValue(value);
            }
            return;
        }
        // 消息框：四按钮对应两正交维的四种组合。
        if (name == _T("msgbox_info") || name == _T("mb_modal_owned")) {
            ShowSampleMessageBox(MessageLevel::Info, MessageButtons::OkCancel, true, true);
            return;
        }
        if (name == _T("toast_info") || name == _T("toast_warn")) {
            ToastSpec spec;
            spec.text = (name == _T("toast_warn"))
                            ? _T("配置已过期，正在自动重新拉取…")
                            : _T("设置已保存。悬浮通知不抢焦点，到时自消。");
            spec.level = (name == _T("toast_warn")) ? MessageLevel::Warning : MessageLevel::Success;
            Toast::Show(m_hWnd, spec);
            return;
        }
        if (name == _T("tip_btn")) {
            Tooltip::Show(m_hWnd, msg.pSender,
                          _T("主题化气泡提示：panel 底、1px 描边，超宽自动折行，到时自消。"));
            return;
        }
        if (name == _T("menu_btn")) {
            // ptMouse 即屏幕坐标（manager 维护的最近鼠标位）。
            std::vector<MenuItem> items = {
                {_T("打开"), 1, true, false},
                {_T("另存为…"), 2, true, true},
                {_T("删除"), 3, true, false},
            };
            const int picked = Menu::Show(m_hWnd, msg.ptMouse, items);
            auto* edit = paint_manager().FindControl(_T("edit"));
            if (edit != nullptr) {
                DuiLib::CDuiString text;
                text.Format(_T("菜单选择 %d"), picked);
                edit->SetText(text.GetData());
            }
            return;
        }
        if (name == _T("mb_modal_free")) {
            ShowSampleMessageBox(MessageLevel::Warning, MessageButtons::YesNo, true, false);
            return;
        }
        if (name == _T("mb_modeless_owned")) {
            ShowSampleMessageBox(MessageLevel::Info, MessageButtons::Ok, false, true);
            return;
        }
        if (name == _T("mb_modeless_free")) {
            ShowSampleMessageBox(MessageLevel::Success, MessageButtons::Ok, false, false);
            return;
        }
    }

    // /msgbox 验证开关：窗口建好后自动弹出示例消息框（截图验收用）。
    // 附加语义：/msgbox_free=模态·独立HWND，/msgbox_modeless=非模态·归属，
    // /msgbox_modeless_free=非模态·独立HWND；/toast=悬浮通知；/tip=气泡提示；
    // /menu=上下文菜单；/date=日历弹层。
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override {
        const LRESULT ret = FramelessWindow::OnCreate(uMsg, wParam, lParam, bHandled);
        if (__argc > 1 && __targv != nullptr) {
            LPCTSTR arg = __targv[1];
            if (_tcsstr(arg, _T("/toast")) != nullptr) {
                ::SetTimer(m_hWnd, kToastTimerId, 250, nullptr);
            } else if (_tcsstr(arg, _T("/tip")) != nullptr) {
                ::SetTimer(m_hWnd, kVerifyTimerId, 250, nullptr);
            } else if (_tcsstr(arg, _T("/menu")) != nullptr) {
                ::SetTimer(m_hWnd, kVerifyTimerId + 1, 250, nullptr);
            } else if (_tcsstr(arg, _T("/date")) != nullptr) {
                ::SetTimer(m_hWnd, kVerifyTimerId + 2, 250, nullptr);
            } else if (_tcsstr(arg, _T("/frame_max")) != nullptr) {
                ::SetTimer(m_hWnd, kVerifyTimerId + 4, 250, nullptr);
            } else if (_tcsstr(arg, _T("/frame")) != nullptr) {
                ::SetTimer(m_hWnd, kVerifyTimerId + 3, 250, nullptr);
            } else if (_tcsstr(arg, _T("/msgbox")) != nullptr) {
                // 一次性定时器触发（WM_TIMER 合并语义，杜绝排队重复）；variant 经
                // 定时器 id 传递：0=模态·归属 1=模态·独立 2=非模态·归属 3=非模态·独立。
                int variant = 0;
                if (_tcsstr(arg, _T("free")) != nullptr) {
                    variant = _tcsstr(arg, _T("modeless")) != nullptr ? 3 : 1;
                } else if (_tcsstr(arg, _T("modeless")) != nullptr) {
                    variant = 2;
                }
                ::SetTimer(m_hWnd, kMsgboxTimerId + variant, 250, nullptr);
            }
        }
        return ret;
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override {
        if (uMsg == WM_TIMER && wParam == kToastTimerId) {
            ::KillTimer(m_hWnd, (UINT_PTR)wParam);
            ToastSpec spec;
            spec.text = _T("设置已保存。悬浮通知不抢焦点，到时自消。");
            spec.level = MessageLevel::Success;
            Toast::Show(m_hWnd, spec);
            return 0;
        }
        if (uMsg == WM_TIMER && wParam >= kVerifyTimerId && wParam < kVerifyTimerId + 5) {
            ::KillTimer(m_hWnd, (UINT_PTR)wParam);
            switch (wParam - kVerifyTimerId) {
                case 0: {
                    auto* target = paint_manager().FindControl(_T("tip_btn"));
                    if (target != nullptr) {
                        Tooltip::Show(m_hWnd, target, _T("主题化气泡提示：panel 底、1px 描边，超宽自动折行。"));
                    }
                    break;
                }
                case 1: {
                    RECT rc = {0, 0, 0, 0};
                    ::GetWindowRect(m_hWnd, &rc);
                    POINT pt{(rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2};
                    std::vector<MenuItem> items = {
                        {_T("打开"), 1, true, false},
                        {_T("另存为…"), 2, true, true},
                        {_T("删除"), 3, true, false},
                    };
                    Menu::Show(m_hWnd, pt, items);  // 阻塞至选择/关闭
                    break;
                }
                case 2: {
                    auto* date = paint_manager().FindControl(_T("date"));
                    if (date != nullptr) {
                        static_cast<DatePicker*>(date)->ShowCalendar();
                    }
                    break;
                }
                case 3:
                    OpenFrameDemo(false);
                    break;
                case 4:
                    OpenFrameDemo(true);
                    break;
            }
            return 0;
        }
        if (uMsg == WM_TIMER && wParam >= kMsgboxTimerId && wParam < kMsgboxTimerId + 4) {
            ::KillTimer(m_hWnd, (UINT_PTR)wParam);
            switch (wParam - kMsgboxTimerId) {
                case 1:
                    ShowSampleMessageBox(MessageLevel::Warning, MessageButtons::YesNo, true, false);
                    break;
                case 2:
                    ShowSampleMessageBox(MessageLevel::Info, MessageButtons::Ok, false, true);
                    break;
                case 3:
                    ShowSampleMessageBox(MessageLevel::Success, MessageButtons::Ok, false, false);
                    break;
                default:
                    ShowSampleMessageBox(MessageLevel::Info, MessageButtons::OkCancel, true, true);
                    break;
            }
            return 0;
        }
        return FramelessWindow::HandleMessage(uMsg, wParam, lParam);
    }

    // /frame(/frame_max)：打开 FrameWindow 预制件演示窗（无主阻塞，自管理）。
    void OpenFrameDemo(bool maximized) {
        auto* frame = new FrameDemoWindow();
        frame->SetTitleText(_T("uikit FrameWindow"));
        const DWORD style = WS_OVERLAPPEDWINDOW;
        HWND hwnd = frame->Create(m_hWnd, _T("uikit FrameWindow"), style, WS_EX_WINDOWEDGE);
        if (hwnd == nullptr) {
            delete frame;
            return;
        }
        frame->ResizeClient(520, 320);
        frame->CenterWindow();
        ::ShowWindow(hwnd, maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
    }

    void ShowSampleMessageBox(MessageLevel level, MessageButtons buttons, bool modal, bool owned) {
        MessageBoxSpec spec;
        spec.title = _T("uikit 消息框");
        spec.text = (level == MessageLevel::Warning)
                        ? _T("配置文件无法解析，是否重建？未保存的修改将丢失。")
                        : _T("这是一条示例消息。模态/非模态 × 归属/独立 HWND 两维正交，可任意组合。");
        spec.level = level;
        spec.buttons = buttons;
        spec.modal = modal;
        spec.owned = owned;
        const MessageResult result = MessageBox::Show(m_hWnd, spec);
        if (modal) {
            auto* edit = paint_manager().FindControl(_T("edit"));
            if (edit != nullptr) {
                DuiLib::CDuiString text;
                text.Format(_T("消息框返回 %d"), static_cast<int>(result));
                edit->SetText(text.GetData());
            }
        }
    }

private:
    RadioButtonUI* radio_light_ = nullptr;
    RadioButtonUI* radio_dark_ = nullptr;
    DuiLib::CListUI* groups_ = nullptr;
    DuiLib::CListUI* items_ = nullptr;
};

}  // namespace

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    ::CoInitialize(nullptr);
    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetCurrentPath(CPaintManagerUI::GetInstancePath());

    DemoWindow* window = new DemoWindow();
    const int code = window->Run(_T("uikit demo"), 560, 740);
    delete window;
    ::CoUninitialize();
    return code;
}
