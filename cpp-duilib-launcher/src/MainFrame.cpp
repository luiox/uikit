#include "MainFrame.h"

using namespace DuiLib;

CControlUI* MainFrame::BuildRootUi() {
    auto* root = new CVerticalLayoutUI();
    root->SetAttribute(_T("bkcolor"), _T("0xFFF7F7F7"));
    root->SetAttribute(_T("bordercolor"), _T("0xFFCCCCCC"));
    root->SetAttribute(_T("bordersize"), _T("1"));

    auto* topBar = new CHorizontalLayoutUI();
    topBar->SetFixedHeight(38);
    topBar->SetAttribute(_T("bkcolor"), _T("0xFFEDEDED"));
    topBar->SetAttribute(_T("childvalign"), _T("vcenter"));
    topBar->SetAttribute(_T("childpadding"), _T("8"));
    topBar->SetAttribute(_T("inset"), _T("10,0,10,0"));

    auto* title = new CLabelUI();
    title->SetText(_T("NAssistant Launcher"));
    title->SetTextColor(0xFF222222);
    title->SetFont(0);
    title->SetFixedWidth(240);
    topBar->Add(title);

    auto* hint = new CLabelUI();
    hint->SetText(_T("Code-built UI (No XML)"));
    hint->SetTextColor(0xFF666666);
    hint->SetAttribute(_T("align"), _T("right"));
    topBar->Add(hint);

    auto* closeBtn = new CButtonUI();
    closeBtn->SetName(_T("closebtn"));
    closeBtn->SetText(_T("Close"));
    closeBtn->SetFixedWidth(70);
    closeBtn->SetFixedHeight(24);
    topBar->Add(closeBtn);

    root->Add(topBar);

    auto* content = new CVerticalLayoutUI();
    content->SetAttribute(_T("inset"), _T("12,12,12,12"));
    content->SetAttribute(_T("childpadding"), _T("8"));

    auto* desc = new CLabelUI();
    desc->SetText(_T("Controls are generated in code for dynamic groups/items and drag import."));
    desc->SetTextColor(0xFF333333);
    content->Add(desc);

    auto* groups = new CListUI();
    groups->SetName(_T("groups_list"));
    groups->SetFixedHeight(220);
    groups->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    groups->SetAttribute(_T("bordercolor"), _T("0xFFE1E1E1"));
    groups->SetAttribute(_T("bordersize"), _T("1"));

    auto* groupItem = new CListLabelElementUI();
    groupItem->SetText(_T("[Sample] Default Group"));
    groups->Add(groupItem);
    content->Add(groups);

    auto* items = new CListUI();
    items->SetName(_T("items_list"));
    items->SetAttribute(_T("bkcolor"), _T("0xFFFFFFFF"));
    items->SetAttribute(_T("bordercolor"), _T("0xFFE1E1E1"));
    items->SetAttribute(_T("bordersize"), _T("1"));

    auto* item1 = new CListLabelElementUI();
    item1->SetText(_T("[Sample] Launch items render dynamically here"));
    items->Add(item1);
    content->Add(items);

    root->Add(content);
    return root;
}

LRESULT MainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    m_pm.Init(m_hWnd, GetManagerName(), this);
    m_pm.AddPreMessageFilter(this);

    CControlUI* root = BuildRootUi();
    if (root == nullptr) {
        MessageBox(nullptr, _T("Create UI failed"), _T("DuiLib"), MB_OK | MB_ICONERROR);
        ExitProcess(1);
        return 0;
    }

    m_pm.AttachDialog(root);
    m_pm.AddNotifier(this);
    __InitWindow();
    bHandled = TRUE;
    return 0;
}

void MainFrame::Notify(TNotifyUI& msg) {
    if (msg.sType == DUI_MSGTYPE_CLICK) {
        if (msg.pSender != nullptr && msg.pSender->GetName() == _T("closebtn")) {
            Close();
            return;
        }
    }
    WindowImplBase::Notify(msg);
}

LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    PostQuitMessage(0);
    bHandled = FALSE;
    return 0;
}
