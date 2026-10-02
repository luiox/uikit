#include "uikit/duilib/datepicker.h"

#include <ctime>

#include "popup_surface.h"
#include "uikit/duilib/compat.h"
#include "uikit/duilib/controls.h"
#include "uikit/duilib/frameless.h"  // CalendarPopup 的基类（头文件只前置声明嵌套类）

namespace uikit {

namespace {

using DuiLib::CDuiRect;
using DuiLib::CDuiSize;
using DuiLib::CDuiString;
using DuiLib::CControlUI;
using DuiLib::TEventUI;
using DuiLib::UIRender;

using DuiLib::UIEVENT_BUTTONDOWN;
using DuiLib::UIEVENT_DBLCLICK;
using DuiLib::UIEVENT_MOUSELEAVE;
using DuiLib::UIEVENT_MOUSEMOVE;
using DuiLib::UIEVENT_SETCURSOR;
using DuiLib::UIEVENT_SETFOCUS;

using uikit::duilib::ButtonUI;
using uikit::duilib::LabelUI;
using uikit::duilib::scale;

// —— 日期数学（每月天数/星期偏移全自算，注意闰年；<ctime> 只用于"今天"）——

bool IsLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

int DaysInMonth(int y, int m) {
    static const int kDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && IsLeapYear(y)) {
        return 29;
    }
    return kDays[m - 1];
}

// 格里高利历外推的序数日（0001-01-01 = 第 1 天），供星期推算——比 mktime
// 少一次往返，也没有 32 位 time_t 的年份范围顾虑。
long long ToOrdinalDay(int y, int m, int d) {
    static const int kAccum[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    const long long years = y - 1;
    long long ordinal = years * 365 + years / 4 - years / 100 + years / 400;
    ordinal += kAccum[m - 1] + d;
    if (m > 2 && IsLeapYear(y)) {
        ordinal += 1;
    }
    return ordinal;
}

// 星期（0=周日 … 6=周六）。锚点校验：2000-01-01=周六、2024-01-01=周一。
int WeekdayOf(int y, int m, int d) {
    return static_cast<int>(ToOrdinalDay(y, m, d) % 7);
}

bool LocalToday(int* y, int* m, int* d) {
    const time_t now = ::time(nullptr);
    std::tm local = {};
#ifdef _MSC_VER
    if (::localtime_s(&local, &now) != 0) {
        return false;
    }
#else
    if (::localtime_r(&now, &local) == nullptr) {
        return false;
    }
#endif
    *y = local.tm_year + 1900;
    *m = local.tm_mon + 1;
    *d = local.tm_mday;
    return true;
}

// 网格几何（96 基准逻辑像素）：7 列 × 6 行，每格 36；星期表头行高 20。
constexpr int kCalendarCell = 36;
constexpr int kWeekdayRowHeight = 20;

}  // namespace

namespace detail {

// 内部件前置声明（定义在弹层之后；弹层持有其指针并授予 friend 访问）。
class CalendarGridUI;
class DatePickerNavButton;

}  // namespace detail

// —— 只读显示框 ————————————————————————————————————————————————

// 包内 CEditUI 没有 readonly 能力（SetAttribute("readonly") 不被包处理、
// SetReadOnly 仅 RichEdit 有），SetEnabled(false) 又会把文字染成禁用色。
// 这里拦掉点击聚焦/光标事件：CEditUI 在 UIEVENT_SETFOCUS 才创建原生 EDIT
// 子窗，事件到不了 DoEvent 基类，原生窗永不创建 = 天然只读，同时保留
// IsEnabled 的正常文本/边框配色。
class DatePicker::ReadOnlyEdit : public Edit {
public:
    ReadOnlyEdit() {
        SetName(_T("dp_display"));  // 宿主可按名识别
    }

    void DoEvent(TEventUI& event) override {
        switch (event.Type) {
            case UIEVENT_BUTTONDOWN:
            case UIEVENT_DBLCLICK:
            case UIEVENT_SETFOCUS:
            case UIEVENT_SETCURSOR:
                return;  // 只读：不进编辑态，不给 I-beam 光标
            default:
                break;
        }
        Edit::DoEvent(event);
    }
};

// —— 日历按钮 ————————————————————————————————————————————————

// 自绘小日历图形：矩形描边 + 顶部两短竖线（装订环）+ 三条横线（文本示意）。
// 全用主题色（border_strong/text_secondary），无 emoji、无图片资产。
class DatePicker::CalendarButton : public ButtonUI {
public:
    explicit CalendarButton(DatePicker* owner) : owner_(owner) {
        SetName(_T("dp_calendar"));
        SetFixedWidth(ActiveTheme().metrics.icon_button_size - 8);
        StyleSecondary();
    }

protected:
    bool Activate() override {
        if (!ButtonUI::Activate()) {
            return false;
        }
        owner_->OpenPopup();
        return true;
    }

    void PaintStatusImage(UIRender* pRender) override {
        ButtonUI::PaintStatusImage(pRender);  // 悬停/按下底（secondary 样式）

        const ResolvedTheme& t = ActiveTheme();
        Color stroke = t.color.border_strong;
        Color lines = t.text_secondary;
        if (!IsEnabled()) {
            stroke = t.control_disabled;
            lines = t.control_disabled;
        }
        const int stroke_px = scale(m_pManager, 1);

        // 日历盒：居中约 52%×52%。
        const int w = m_rcItem.right - m_rcItem.left;
        const int h = m_rcItem.bottom - m_rcItem.top;
        const int bw = w * 52 / 100;
        const int bh = h * 52 / 100;
        const int bl = m_rcItem.left + (w - bw) / 2;
        const int bt = m_rcItem.top + (h - bh) / 2;

        CDuiRect box;
        box.left = bl;
        box.top = bt;
        box.right = bl + bw;
        box.bottom = bt + bh;
        pRender->DrawRect(box, stroke_px, stroke);

        // 顶部两短竖线（装订环），略越过盒顶沿。
        const int ring_top = bt - (h * 5 / 100 + 1);
        const int ring_bottom = bt + bh * 25 / 100;
        pRender->DrawLine(bl + bw * 30 / 100, ring_top, bl + bw * 30 / 100, ring_bottom,
                          stroke_px, stroke);
        pRender->DrawLine(bl + bw * 70 / 100, ring_top, bl + bw * 70 / 100, ring_bottom,
                          stroke_px, stroke);

        // 三条横线示意文本行。
        const int line_l = bl + bw * 18 / 100;
        const int line_r = bl + bw * 82 / 100;
        for (int i = 1; i <= 3; ++i) {
            const int y = bt + bh * (20 + i * 18) / 100;
            pRender->DrawLine(line_l, y, line_r, y, stroke_px, lines);
        }
    }

private:
    DatePicker* owner_;
};

// —— 日历弹层 ————————————————————————————————————————————————

// toast 配方（NOACTIVATE + TOOLWINDOW）的日历弹层。焦点取舍：
// · 保持 NOACTIVATE——点选交互纯鼠标即可，焦点留在宿主（正在输入的场景不被
//   打断）；duilib 的点击链路会在按下时强设弹层焦点（manager OnLButtonDown
//   恒 SetFocus），CloseSelf 里检测并归还宿主，兑现"不扰动焦点"。
// · 点外部关闭 = SetCapture 捕获"按下点在窗外"；只拦按下不拦抬起（弹出瞬间
//   回放的旧 WM_LBUTTONUP 不误关）。NOACTIVATE 窗口没有键盘焦点，Esc 关闭
//   做不到——守住"能收起"的底线（点外部/失焦/选日均收起），不做半吊子的
//   临时抢焦点方案。
// 成员函数体在 detail 内部件定义之后出行实现（BuildRootUi 要 new 它们）。
class DatePicker::CalendarPopup : private duilib::FramelessWindow {
    friend class detail::CalendarGridUI;       // 网格读视图状态/回调选日
    friend class detail::DatePickerNavButton;  // 翻月钮回调 ShiftMonth

public:
    CalendarPopup();

    // 同步选中日期/今天快照 → 建窗 → 定位 → 显示 → 捕获鼠标。
    // 失败自清哨兵并自删（内部件不出 cpp，生命周期全自管理）。
    void OpenAt(DatePicker* owner);

    // 关闭自管理（toast 同款）：先断 owner 回指、归还焦点，再异步销毁。
    void CloseSelf();

private:
    LPCTSTR GetWindowClassName() const override;
    DuiLib::CControlUI* BuildRootUi() override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    void OnFinalMessage(HWND hWnd) override;
    // 弹层内条目（翻月钮/格子）经 Activate/DoEvent 自持，不经窗口 Notify。
    void Notify(DuiLib::TNotifyUI& msg) override { (void)msg; }

    // 网格/翻月钮回调与内部工具（detail 内部件经 friend 访问）。
    int FirstWeekday() const;
    int DaysInViewMonth() const;
    void ShiftMonth(int delta);
    void ConfirmDay(int day);
    void RefreshTitle();
    void Fail(DatePicker* owner);

    DatePicker* owner_ = nullptr;
    uikit::duilib::LabelUI* title_ = nullptr;
    detail::CalendarGridUI* grid_ = nullptr;
    // 选中日期（打开时快照，弹层内只读）与视图年月（翻页可变）。
    int sel_year_ = 0;
    int sel_month_ = 0;
    int sel_day_ = 0;
    int view_year_ = 0;
    int view_month_ = 0;
    int today_year_ = 0;
    int today_month_ = 0;
    int today_day_ = 0;
    bool closing_ = false;
};

namespace detail {

// —— 日历网格（42 格自绘） ——————————————————————————————————————

// 一次 Paint 画完 6×7=42 格（自绘一格控件，不摆 42 个子控件）：今天 = accent
// 1px 描边，选中日 = accent 底 + on_accent 文字，悬停 surface_hover，相邻月
// 补位格弱化色。悬停/点选只认本月格。
class CalendarGridUI : public DuiLib::CControlUI {
public:
    explicit CalendarGridUI(DatePicker::CalendarPopup* popup) : popup_(popup) {}

    // 月份切换时复位悬停指向（格位对应的日子已变）。
    void ResetHover() { hover_index_ = -1; }

    void DoEvent(TEventUI& event) override {
        if (event.Type == UIEVENT_MOUSEMOVE) {
            const int index = CellAt(event.ptMouse);
            if (index != hover_index_) {
                hover_index_ = index;
                Invalidate();
            }
        } else if (event.Type == UIEVENT_MOUSELEAVE) {
            if (hover_index_ >= 0) {
                hover_index_ = -1;
                Invalidate();
            }
        } else if (event.Type == UIEVENT_BUTTONDOWN) {
            // 按下选日（任务口径）；CellAt 只认本月格。
            const int index = CellAt(event.ptMouse);
            if (index >= 0) {
                popup_->ConfirmDay(index + 1 - popup_->FirstWeekday());
                return;
            }
        }
        CControlUI::DoEvent(event);
    }

    bool Paint(UIRender* pRender, const CDuiRect& rcPaint,
               CControlUI* pStopControl) override;

private:
    // 命中测试：返回网格索引（0..41）或 -1（格外/非本月格不可选不悬停）。
    int CellAt(const DuiLib::CDuiPoint& pt) const {
        if (!m_rcItem.PtInRect(pt)) {
            return -1;
        }
        const int width = m_rcItem.right - m_rcItem.left;
        const int height = m_rcItem.bottom - m_rcItem.top;
        const int col = (pt.x - m_rcItem.left) * 7 / width;
        const int row = (pt.y - m_rcItem.top) * 6 / height;
        if (col < 0 || col > 6 || row < 0 || row > 5) {
            return -1;
        }
        const int index = row * 7 + col;
        const int day = index + 1 - popup_->FirstWeekday();
        if (day < 1 || day > popup_->DaysInViewMonth()) {
            return -1;
        }
        return index;
    }

    DatePicker::CalendarPopup* popup_;
    int hover_index_ = -1;
};

// —— 翻月按钮（自绘三角箭头） ——————————————————————————————————

class DatePickerNavButton : public ButtonUI {
public:
    DatePickerNavButton(DatePicker::CalendarPopup* popup, int direction)
        : popup_(popup), direction_(direction) {
        SetName(direction_ < 0 ? _T("dp_prev_month") : _T("dp_next_month"));
        SetFixedWidth(ActiveTheme().metrics.control_height);  // 近方形
        StyleSecondary();
    }

protected:
    bool Activate() override {
        if (!ButtonUI::Activate()) {
            return false;
        }
        popup_->ShiftMonth(direction_);
        return true;
    }

    void PaintStatusImage(UIRender* pRender) override {
        ButtonUI::PaintStatusImage(pRender);  // 悬停/按下底

        const ResolvedTheme& t = ActiveTheme();
        Color stroke = t.text_secondary;
        if (!IsEnabled()) {
            stroke = t.control_disabled;
        } else if (IsHotState() || IsPushedState()) {
            stroke = t.color.text;
        }
        const int stroke_px = scale(m_pManager, 2);  // 与 ComboBox 箭头同宽

        // 箭头取控件中央 30%×40% 区域的两条线（‹ 或 ›）。
        const int w = m_rcItem.right - m_rcItem.left;
        const int h = m_rcItem.bottom - m_rcItem.top;
        const int cx = m_rcItem.left + w / 2;
        const int cy = m_rcItem.top + h / 2;
        const int half_span = h * 20 / 100;
        const int half_width = w * 15 / 100;
        if (direction_ < 0) {
            pRender->DrawLine(cx + half_width, cy - half_span, cx - half_width, cy,
                              stroke_px, stroke);
            pRender->DrawLine(cx - half_width, cy, cx + half_width, cy + half_span,
                              stroke_px, stroke);
        } else {
            pRender->DrawLine(cx - half_width, cy - half_span, cx + half_width, cy,
                              stroke_px, stroke);
            pRender->DrawLine(cx + half_width, cy, cx - half_width, cy + half_span,
                              stroke_px, stroke);
        }
    }

private:
    DatePicker::CalendarPopup* popup_;
    int direction_;
};

}  // namespace detail

// —— CalendarPopup 成员函数（detail 内部件已完整，可安全 new） ————————

DatePicker::CalendarPopup::CalendarPopup() {
    // 弹层无缩放/无拖拽区：caption 缺省 30px 会让顶部网格行变成 HTCAPTION
    // 拖拽区（Toast/MessageBox 同款归零）。
    sizebox_px_ = 0;
    caption_h_px_ = 0;
}

void DatePicker::CalendarPopup::OpenAt(DatePicker* owner) {
    HWND host = nullptr;
    if (owner != nullptr && owner->GetManager() != nullptr) {
        host = owner->GetManager()->GetPaintWindow();
        sel_year_ = owner->year_;
        sel_month_ = owner->month_;
        sel_day_ = owner->day_;
        view_year_ = owner->year_;
        view_month_ = owner->month_;
    }
    // "今天"打开时取快照：跨午夜不漂移，重开时重新取。
    if (!LocalToday(&today_year_, &today_month_, &today_day_)) {
        today_year_ = today_month_ = today_day_ = 0;  // 取不到则不画今天环
    }
    if (host == nullptr) {
        Fail(owner);
        return;
    }
    // NOACTIVATE：不抢宿主焦点；TOOLWINDOW：不进任务栏/Alt-Tab；TOPMOST 压顶。
    const DWORD ex_style =
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_WINDOWEDGE;
    HWND hwnd = Create(host, _T(""), WS_POPUP, ex_style);
    if (hwnd == nullptr) {
        Fail(owner);
        return;
    }

    // 尺寸：宽 = 7×36 + 两侧 window_inset；高 = 头行 + 星期行 + 6×36 网格
    // + 两侧 inset + 两道 childpadding（4）。
    const ResolvedTheme& t = ActiveTheme();
    const int width_logical = 7 * kCalendarCell + t.metrics.window_inset * 2;
    const int height_logical = t.metrics.window_inset * 2 + t.metrics.control_height +
                               kWeekdayRowHeight + 6 * kCalendarCell + 8;
    ResizeClient(scale(&m_pm, width_logical), scale(&m_pm, height_logical));

    // 位置：贴控件下缘左对齐展开；放不下翻到控件上方，横向夹进工作区。
    const CDuiRect rc = owner->GetPos();  // 宿主客户区坐标
    POINT anchor_tl = {rc.left, rc.top};
    POINT anchor_bl = {rc.left, rc.bottom};
    ::ClientToScreen(host, &anchor_tl);
    ::ClientToScreen(host, &anchor_bl);
    RECT rc_work = {anchor_bl.x, anchor_bl.y, anchor_bl.x + 1, anchor_bl.y + 1};
    if (const HMONITOR monitor = ::MonitorFromWindow(host, MONITOR_DEFAULTTONEAREST)) {
        MONITORINFO mi = {sizeof(mi)};
        if (::GetMonitorInfo(monitor, &mi)) {
            rc_work = mi.rcWork;
        }
    }
    const int w = scale(&m_pm, width_logical);
    const int h = scale(&m_pm, height_logical);
    int x = anchor_tl.x;
    int y = anchor_bl.y;
    if (x + w > rc_work.right) {
        x = rc_work.right - w;
    }
    if (x < rc_work.left) {
        x = rc_work.left;
    }
    if (y + h > rc_work.bottom) {
        y = anchor_tl.y - h;  // 上翻：贴控件上缘
    }
    if (y < rc_work.top) {
        y = rc_work.top;
    }
    ::SetWindowPos(hwnd, HWND_TOPMOST, x, y, 0, 0,
                   SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    // 捕获鼠标：外点关闭的落点（见 HandleMessage）。触发本弹层的宿主按钮已在
    // BUTTONUP 里先释放捕获再 Activate（fork CButtonUI::DoEvent 顺序），不会
    // 被随后的 ReleaseCapture 抹掉。
    ::SetCapture(hwnd);
    owner_ = owner;  // 全部就绪后才回指：失败路径不依赖它
}

void DatePicker::CalendarPopup::CloseSelf() {
    if (closing_) {
        return;
    }
    closing_ = true;
    if (owner_ != nullptr) {
        // duilib 点击链路把焦点强设到了弹层（manager OnLButtonDown 恒
        // SetFocus），收起时还给宿主。（SetFocus 会向弹层发 WM_KILLFOCUS，
        // closing_ 已置位防重入。）
        if (owner_->GetManager() != nullptr) {
            const HWND host = owner_->GetManager()->GetPaintWindow();
            if (::GetFocus() == m_hWnd && host != nullptr && ::IsWindow(host)) {
                ::SetFocus(host);
            }
        }
        owner_->popup_ = nullptr;
        owner_ = nullptr;
    }
    Close(0);
}

LPCTSTR DatePicker::CalendarPopup::GetWindowClassName() const {
    return _T("UIKitDatePickerPopup");
}

DuiLib::CControlUI* DatePicker::CalendarPopup::BuildRootUi() {
    const ResolvedTheme& t = ActiveTheme();
    auto* root = new uikit::detail::PopupSurfaceUI();
    CDuiString inset;
    inset.Format(_T("%d,%d,%d,%d"), t.metrics.window_inset, t.metrics.window_inset,
                 t.metrics.window_inset, t.metrics.window_inset);
    root->SetAttribute(_T("inset"), inset.GetData());
    root->SetAttribute(_T("childpadding"), _T("4"));

    // 顶行：‹ 上月 / "YYYY年M月" / 下月 ›。
    auto* header = new DuiLib::CHorizontalLayoutUI();
    header->SetFixedHeight(t.metrics.control_height);
    header->Add(new detail::DatePickerNavButton(this, -1));
    title_ = new LabelUI();
    title_->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    header->Add(title_);
    header->Add(new detail::DatePickerNavButton(this, 1));
    root->Add(header);
    RefreshTitle();

    // 星期表头（7 列，列宽对齐网格，弱化色）。周日打头，与 WeekdayOf 一致。
    auto* week = new DuiLib::CHorizontalLayoutUI();
    week->SetFixedHeight(kWeekdayRowHeight);
    static const LPCTSTR kWeekdays[7] = {_T("日"), _T("一"), _T("二"),
                                         _T("三"), _T("四"), _T("五"), _T("六")};
    for (const LPCTSTR weekday : kWeekdays) {
        auto* label = new LabelUI();
        label->SetRole(LabelUI::Role::Secondary);
        label->SetText(weekday);
        label->SetFixedWidth(kCalendarCell);
        label->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        week->Add(label);
    }
    root->Add(week);

    grid_ = new detail::CalendarGridUI(this);
    grid_->SetFixedHeight(6 * kCalendarCell);
    root->Add(grid_);
    return root;
}

LRESULT DatePicker::CalendarPopup::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            // 捕获后所有按下都进弹层：窗外按下 = 点外部 → 收起。
            const DuiLib::CDuiPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            RECT rc_win = {0, 0, 0, 0};
            ::GetWindowRect(m_hWnd, &rc_win);
            if (pt.x < 0 || pt.y < 0 || pt.x >= rc_win.right - rc_win.left ||
                pt.y >= rc_win.bottom - rc_win.top) {
                CloseSelf();
                return 0;
            }
            break;
        }
        case WM_KILLFOCUS:
            // 焦点被第三方抢走（点击其他应用等）：随之收起。
            CloseSelf();
            break;
        case WM_CAPTURECHANGED:
            // 捕获被抢（罕见）：视同失焦收起。
            CloseSelf();
            break;
        default:
            break;
    }
    return duilib::FramelessWindow::HandleMessage(uMsg, wParam, lParam);
}

void DatePicker::CalendarPopup::OnFinalMessage(HWND) {
    delete this;  // 自管理生命周期：DatePicker 只持"已开"哨兵指针
}

int DatePicker::CalendarPopup::FirstWeekday() const {
    return WeekdayOf(view_year_, view_month_, 1);
}

int DatePicker::CalendarPopup::DaysInViewMonth() const {
    return DaysInMonth(view_year_, view_month_);
}

void DatePicker::CalendarPopup::ShiftMonth(int delta) {
    view_month_ += delta;
    if (view_month_ > 12) {
        view_month_ = 1;
        ++view_year_;
    }
    if (view_month_ < 1) {
        view_month_ = 12;
        --view_year_;
    }
    if (view_year_ < 1) {
        view_year_ = 1;
        view_month_ = 1;
    }
    if (view_year_ > 9999) {
        view_year_ = 9999;
        view_month_ = 12;
    }
    RefreshTitle();
    if (grid_ != nullptr) {
        grid_->ResetHover();  // 月份变了，悬停格指向失效
        grid_->Invalidate();
    }
}

void DatePicker::CalendarPopup::ConfirmDay(int day) {
    if (owner_ != nullptr) {
        owner_->ConfirmDay(view_year_, view_month_, day);  // 通知后经 ClosePopup 收起
    } else {
        CloseSelf();
    }
}

void DatePicker::CalendarPopup::RefreshTitle() {
    if (title_ != nullptr) {
        CDuiString title;
        title.Format(_T("%d年%d月"), view_year_, view_month_);
        title_->SetText(title.GetData());
    }
}

void DatePicker::CalendarPopup::Fail(DatePicker* owner) {
    if (owner != nullptr) {
        owner->popup_ = nullptr;
    }
    delete this;
}

// —— 日历网格绘制 ——————————————————————————————————————————————

bool detail::CalendarGridUI::Paint(UIRender* pRender, const CDuiRect& rcPaint,
                                   CControlUI* pStopControl) {
    const ResolvedTheme& t = ActiveTheme();
    const int first_weekday = popup_->FirstWeekday();
    const int days = popup_->DaysInViewMonth();
    // 相邻月补位格的显示基准（上月天数，跨年已折算）。
    const int prev_month = (popup_->view_month_ > 1) ? popup_->view_month_ - 1 : 12;
    const int prev_month_year =
        (popup_->view_month_ > 1) ? popup_->view_year_ : popup_->view_year_ - 1;
    const int prev_month_days = DaysInMonth(prev_month_year, prev_month);
    const bool today_in_view = popup_->today_year_ == popup_->view_year_ &&
                               popup_->today_month_ == popup_->view_month_;
    const int selected_index =
        (popup_->sel_year_ == popup_->view_year_ && popup_->sel_month_ == popup_->view_month_)
            ? popup_->sel_day_ - 1 + first_weekday
            : -1;

    const int width = m_rcItem.right - m_rcItem.left;
    const int height = m_rcItem.bottom - m_rcItem.top;
    const int stroke_px = scale(m_pManager, 1);
    const int radius = scale(m_pManager, t.metrics.radius_control);

    for (int index = 0; index < 42; ++index) {
        const int day = index + 1 - first_weekday;  // <1 或 >days = 相邻月补位
        const bool in_month = day >= 1 && day <= days;
        const int col = index % 7;
        const int row = index / 7;

        // 精确均分：每格边界按比例取整，右侧/底部不留残缝。
        CDuiRect cell;
        cell.left = m_rcItem.left + width * col / 7;
        cell.right = m_rcItem.left + width * (col + 1) / 7;
        cell.top = m_rcItem.top + height * row / 6;
        cell.bottom = m_rcItem.top + height * (row + 1) / 6;

        // 底：选中 = accent 实底；悬停 = surface_hover（仅本月格）。
        if (index == selected_index) {
            pRender->DrawColor(cell, CDuiSize(radius, radius), t.color.accent);
        } else if (in_month && index == hover_index_) {
            pRender->DrawColor(cell, CDuiSize(radius, radius), t.color.surface_hover);
        }

        // 今天 = accent 1px 描边（与选中底并存，语义不冲突）。
        if (today_in_view && day == popup_->today_day_) {
            CDuiRect ring = cell;
            ::InflateRect(&ring, -stroke_px * 2, -stroke_px * 2);
            pRender->DrawRoundRect(ring, stroke_px, CDuiSize(radius, radius),
                                   t.color.accent);
        }

        // 文本：本月正文色，相邻月补位格弱化色（日期号折回相邻月真实日），
        // 选中格 on_accent。
        int shown = day;
        if (day < 1) {
            shown = day + prev_month_days;
        } else if (day > days) {
            shown = day - days;
        }
        CDuiString text;
        text.Format(_T("%d"), shown);
        Color text_color = in_month ? t.color.text : t.text_secondary;
        if (index == selected_index) {
            text_color = t.color.on_accent;
        }
        pRender->DrawText(cell, text.GetData(), text_color, 0,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    return CControlUI::Paint(pRender, rcPaint, pStopControl);
}

// —— 复合本体 —————————————————————————————————————————————————

DatePicker::DatePicker() {
    const ResolvedTheme& t = ActiveTheme();
    SetFixedHeight(t.metrics.control_height);
    SetAttribute(_T("childpadding"), _T("2"));

    edit_ = new ReadOnlyEdit();
    Add(edit_);
    button_ = new CalendarButton(this);
    Add(button_);

    // 默认落今天：日期控件最常见的初始语义（用户可直接确认或改）。
    int y = 0;
    int m = 0;
    int d = 0;
    if (LocalToday(&y, &m, &d)) {
        year_ = y;
        month_ = m;
        day_ = d;
    }
    ApplyDateText();
}

void DatePicker::SetDate(int y, int m, int d) {
    // 非法分量钳位：月份收进 1..12，"日"按当月天数收口（含闰年二月）。
    year_ = (y < 1) ? 1 : (y > 9999 ? 9999 : y);
    month_ = (m < 1) ? 1 : (m > 12 ? 12 : m);
    const int max_day = DaysInMonth(year_, month_);
    day_ = (d < 1) ? 1 : (d > max_day ? max_day : d);
    ApplyDateText();
    // 已开的弹层是独立视图快照（收起再开才同步）；SetDate 属程序设值，不通知。
}

void DatePicker::ApplyDateText() {
    if (edit_ == nullptr) {
        return;
    }
    CDuiString text;
    text.Format(_T("%04d-%02d-%02d"), year_, month_, day_);
    edit_->SetText(text.GetData());
}

void DatePicker::OpenPopup() {
    if (popup_ != nullptr) {
        return;  // 已开（按钮二次点击会被弹层捕获路径收起，走不到这里）
    }
    auto* popup = new CalendarPopup();
    popup_ = popup;        // 先挂哨兵，OpenAt 失败时经 Fail() 清回
    popup->OpenAt(this);
}

void DatePicker::ConfirmDay(int y, int m, int d) {
    year_ = y;
    month_ = m;
    day_ = d;
    ApplyDateText();
    if (GetManager() != nullptr) {
        GetManager()->SendNotify(this, _T("datechange"),
                                 static_cast<WPARAM>(GetDateYmd()), 0);
    }
    ClosePopup();
}

void DatePicker::ClosePopup() {
    if (popup_ != nullptr) {
        popup_->CloseSelf();  // 内部断回指/归还焦点/异步销毁
    }
}

}  // namespace uikit
