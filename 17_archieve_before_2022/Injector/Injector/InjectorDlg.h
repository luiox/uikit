
// InjectorDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"


// CInjectorDlg 对话框
class CInjectorDlg : public CDialogEx
{
// 构造
public:
	CInjectorDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_INJECTOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_list;           //列表控件
	void BrowserProcess();      //遍历进程
	afx_msg void OnBnClickedBtnCloseprocess();
	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnBnClickedBtnExit();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedBtnInject();
	afx_msg void OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_strDllPath;
	afx_msg void OnBnClickedBtnChoice();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedBtnAbout();
};
