#pragma once

// 日期选择（L3，uikit 顶层命名空间）：只读显示框 + 日历按钮的复合控件，
// 点击按钮弹 FramelessWindow 日历弹层（toast 配方：NOACTIVATE + TOOLWINDOW，
// 不抢宿主焦点、不进任务栏）。

#include <UIlib.h>

#include "uikit/duilib/controls.h"
#include "uikit/duilib/edit.h"

namespace uikit {

// 布局：[只读 Edit][日历按钮]（spinbox 的复合 + 内部子类拦截同款模式）。
// · Edit 只读：包内 CEditUI 无 readonly 能力（SetAttribute("readonly") 不被
//   包处理、SetReadOnly 仅 RichEdit 有）——用内部子类拦掉点击聚焦/光标事件，
//   原生 EDIT 子窗从不创建，即天然只读，且保持 IsEnabled 的正常配色。
// · 日历按钮：内部子类（宽 icon_button_size-8），自绘日历图形（矩形描边 +
//   顶部两短竖线 + 三条横线示意文本，border_strong/text_secondary，无 emoji
//   无图片）；点击 → 日历弹层。
// · 弹层：顶行 ‹ 上月 / "YYYY年M月" / 下月 ›（箭头自绘两条线），星期表头一行
//   7 列，6×7=42 格网格（每格 36 逻辑像素，绘制期按 DPI scale）。今天 =
//   accent 1px 描边，选中日 = accent 底 + on_accent 文字，悬停 surface_hover，
//   非本月日期走弱化色。翻页只改视图不通知；**确认选中某日**才向窗口发
//   "datechange"（wParam = yyyymmdd）并收起弹层。
// · 弹层 NOACTIVATE（点选交互纯鼠标，焦点留在宿主减少扰动）：点外部关闭靠
//   SetCapture 捕获外点按下；Esc 需键盘焦点、NOACTIVATE 收不到，故不支持
//   （点按钮/点外部即收起，取舍详见 CalendarPopup 注释）。
// · 日期数学只用 <ctime>：星期偏移由 mktime 规范化回填 tm_wday 求得，每月
//   天数/闰年自算，无第三方依赖。
class DatePicker : public DuiLib::CHorizontalLayoutUI, public duilib::ThemeableControl {
public:
    DatePicker();  // 默认落今天：日期控件最常见的初始语义

    LPCTSTR GetClass() const override { return _T("UIKitDatePicker"); }
    // 本体不自绘；Edit/按钮均为 ThemeableControl，ApplyThemeToTree 逐个重涂
    //（spinbox 同款）。
    void OnThemeChanged() override {}

    // 非法分量按当月天数/年份范围钳位（月份 1..12，日 1..当月天数）。
    void SetDate(int y, int m, int d);
    // 取值给三参分解接口：与 SetDate 对称，消费方免拆包；yyyymmdd 打包只用于
    // "datechange" 通知的单值通道（wParam）。
    int GetYear() const { return year_; }
    int GetMonth() const { return month_; }
    int GetDay() const { return day_; }
    int GetDateYmd() const { return year_ * 10000 + month_ * 100 + day_; }

    // 嵌套前置声明（全部实现于 datepicker.cpp，不暴露任何构造/成员）：类型名
    // 公开只是让 cpp 内的内部件（弹层/网格/按钮）能互相命名引用。
    class CalendarButton;
    class ReadOnlyEdit;
    class CalendarPopup;

private:
    friend class CalendarButton;
    friend class CalendarPopup;

    void OpenPopup();
    // 弹层确认回调：更新显示 + 发通知 + 收起弹层（子类行/格子直接调）。
    void ConfirmDay(int y, int m, int d);
    void ClosePopup();
    void ApplyDateText();

    ReadOnlyEdit* edit_ = nullptr;
    CalendarButton* button_ = nullptr;
    int year_ = 2000;
    int month_ = 1;
    int day_ = 1;
    // 非空 = 弹层开着。弹层关闭自删（toast 同款 OnFinalMessage），本指针只作
    // "已开"哨兵；弹层与宿主同线程同生命周期，不存在跨线程悬垂。
    CalendarPopup* popup_ = nullptr;
};

}  // namespace uikit
