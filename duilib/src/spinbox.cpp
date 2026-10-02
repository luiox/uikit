#include "uikit/duilib/spinbox.h"

#include "uikit/duilib/controls.h"

namespace uikit {

using DuiLib::CDuiString;
using DuiLib::TNotifyUI;

// 内部减/加钮：点击直接回调宿主 SpinBox（粘滞态/主题底由 ButtonUI 承接）。
class SpinBox::StepButton : public duilib::ButtonUI {
public:
    StepButton(SpinBox* owner, LPCTSTR text, LPCTSTR name, int direction)
        : owner_(owner), direction_(direction) {
        SetName(name);  // 宿主可按名识别（sb_dec/sb_inc）
        SetText(text);
        SetFixedWidth(ActiveTheme().metrics.icon_button_size - 4);
        StyleSecondary();
    }

protected:
    bool Activate() override {
        if (!ButtonUI::Activate()) {
            return false;
        }
        owner_->Step(direction_);
        return true;
    }

private:
    SpinBox* owner_;
    int direction_;
};

SpinBox::SpinBox() {
    const ResolvedTheme& t = ActiveTheme();
    SetFixedHeight(t.metrics.control_height);
    SetAttribute(_T("childpadding"), _T("4"));

    auto* dec = new StepButton(this, _T("−"), _T("sb_dec"), -1);
    Add(dec);
    edit_ = new Edit();
    edit_->SetNumberOnly(true);
    Add(edit_);
    auto* inc = new StepButton(this, _T("+"), _T("sb_inc"), 1);
    Add(inc);

    // 手输同步：Edit 的 return/killfocus 经控件级 OnNotify 挂点进来。
    edit_->OnNotify += MakeDelegate(this, &SpinBox::OnChildNotify);

    ApplyValue();
}

void SpinBox::OnThemeChanged() {
    // 子控件（Edit/按钮）均为 ThemeableControl，ApplyThemeToTree 会逐个重涂；
    // 本体无自绘内容，无需额外处理。
}

void SpinBox::SetValue(int value) {
    if (value < min_value_) {
        value = min_value_;
    }
    if (value > max_value_) {
        value = max_value_;
    }
    value_ = value;
    ApplyValue();
}

void SpinBox::SetValueRange(int min_value, int max_value) {
    min_value_ = min_value;
    max_value_ = (max_value >= min_value) ? max_value : min_value;
    SetValue(value_);
}

void SpinBox::Step(int direction) {
    SetValue(value_ + direction * step_);
    if (GetManager() != nullptr) {
        GetManager()->SendNotify(this, _T("valuechange"), static_cast<WPARAM>(value_), 0);
    }
}

void SpinBox::ApplyValue() {
    if (edit_ == nullptr) {
        return;
    }
    CDuiString text;
    text.Format(_T("%d"), value_);
    edit_->SetText(text.GetData());
}

bool SpinBox::OnChildNotify(void* param) {
    auto* msg = static_cast<TNotifyUI*>(param);
    if (edit_ == nullptr || msg == nullptr || msg->pSender != edit_) {
        return false;
    }
    if (msg->sType == _T("return") || msg->sType == _T("killfocus")) {
        SetValue(_ttoi(edit_->GetText().GetData()));
    }
    return false;
}

}  // namespace uikit
