// uikit demo：主题（深/浅实时切换）+ 统一控件目检窗口。
// 纯代码构建（与 mlaunch 同款做法），无 XML 资产依赖。

#include <UIlib.h>

#include "uikit/duilib/applier.h"
#include "uikit/duilib/controls.h"
#include "uikit/duilib/frameless.h"

using namespace DuiLib;
using namespace uikit;
using namespace uikit::duilib;

namespace {

DuiLib::CControlUI* MakeFixedSpacer(int height) {
    auto* spacer = new DuiLib::CControlUI();
    spacer->SetFixedHeight(height);
    return spacer;
}

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
    const int code = window->Run(_T("uikit demo"), 560, 440);
    delete window;
    ::CoUninitialize();
    return code;
}
