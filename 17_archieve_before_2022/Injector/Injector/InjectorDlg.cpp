
// InjectorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Injector.h"
#include "InjectorDlg.h"
#include "afxdialogex.h"
#include <tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInjectorDlg 对话框



CInjectorDlg::CInjectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInjectorDlg::IDD, pParent)
	, m_strDllPath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CInjectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_EDIT1, m_strDllPath);
	DDV_MaxChars(pDX, m_strDllPath, 255);
}

BEGIN_MESSAGE_MAP(CInjectorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CLOSEPROCESS, &CInjectorDlg::OnBnClickedBtnCloseprocess)
	ON_BN_CLICKED(IDC_BTN_REFRESH, &CInjectorDlg::OnBnClickedBtnRefresh)
	ON_BN_CLICKED(IDC_BTN_EXIT, &CInjectorDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_INJECT, &CInjectorDlg::OnBnClickedBtnInject)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CInjectorDlg::OnRclickList1)
	ON_BN_CLICKED(IDC_BTN_CHOICE, &CInjectorDlg::OnBnClickedBtnChoice)
	ON_BN_CLICKED(IDC_BTN_ABOUT, &CInjectorDlg::OnBnClickedBtnAbout)
END_MESSAGE_MAP()


// CInjectorDlg 消息处理程序

BOOL CInjectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//加载图标
	//HICON hIcon=::LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON1));
	//HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	//设置图标
	//SetIcon(hIcon, TRUE);		
	//SetIcon(hIcon, FALSE);

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	//给列表添加列    LVCFMT_LEFT：表头风格，左对齐
	m_list.InsertColumn(0, _T("编号"), LVCFMT_LEFT,100);
	m_list.InsertColumn(1, _T("进程名称"), LVCFMT_LEFT,200);
	m_list.InsertColumn(2, _T("PID"), LVCFMT_LEFT, 100);
	m_list.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);//设置风格

	BrowserProcess();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CInjectorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CInjectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//遍历进程
void CInjectorDlg::BrowserProcess()
{
	//创建进程快照
	HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);//预留空间

	BOOL bMore = Process32First(hProcessSnap, &pe32);
	//查找全部进程
	int i = 0;
	CString str;
	while (bMore)
	{
		str.Format(_T("%d"), (i+1));               //格式化字符
		m_list.InsertItem(i, str);                 //设置编号
		m_list.SetItemText(i, 1, pe32.szExeFile);  //设置进程名称

		str.Format(_T("%d"), pe32.th32ProcessID);  //格式化字符
		m_list.SetItemText(i, 2, str);             //设置PID
		
		bMore=Process32Next(hProcessSnap, &pe32);
		i++;
	}


}

//结束进程
void CInjectorDlg::OnBnClickedBtnCloseprocess()
{
	//获取选中的进程PID
	int nSel=m_list.GetSelectionMark();
	if (nSel < 0)
		return;

	CString strPID=m_list.GetItemText(nSel, 2);
	DWORD dwPID=_wtoi(strPID);

	//打开进程
	HANDLE hProcess=OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (hProcess == NULL)
	{
		MessageBox(L"打开进程失败");
		return;
	}

	//关闭进程
	TerminateProcess(hProcess,0);

	Sleep(2000);
	//刷新列表
	OnBnClickedBtnRefresh();

}

//刷新列表
void CInjectorDlg::OnBnClickedBtnRefresh()
{
	//重新创建快照
	m_list.DeleteAllItems();
	BrowserProcess();
}

//退出
void CInjectorDlg::OnBnClickedBtnExit()
{
	exit(0);
}


//注入DLL
void CInjectorDlg::OnBnClickedBtnInject()
{
	DWORD dllPathSize = ((DWORD)_tcslen(m_strDllPath) + 1) * sizeof(TCHAR);
	//准备注入
	UpdateData(TRUE);
	
	//判断m_strDllPath是否有内容
	if (m_strDllPath.IsEmpty())
	{
		MessageBox(L"没有选择模块\n请选择需要注入的DLL模块", L"提示");
	}
	
	//获取进程PID
	int nSel = m_list.GetSelectionMark();
	if (nSel < 0)
		return;

	CString strPID = m_list.GetItemText(nSel, 2);
	DWORD dwPID = _wtoi(strPID);
	
	//通过PID获取进程句柄
	HANDLE hProcess=OpenProcess(PROCESS_ALL_ACCESS, FALSE,dwPID);
	if (hProcess == NULL)
	{
		MessageBox(L"获取进程句柄失败");
		return;
	}

	//开始注入
	//提升权限
	//1.打开进程访问令牌
	HANDLE hToken;
	if (FALSE == OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		MessageBox(L"打开进程的访问令牌失败");
		return;
	}
	
	//2.查看与进程相关的特权信息
	LUID luid;
	if (FALSE == LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
	{
		MessageBox(L"查看与进程相关的特权信息失败");
		return;
	}
	
	//3.调节进程的访问令牌特权属性
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (FALSE == AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		MessageBox(L"提升权限失败");
		return;
	}
	
	//在远程进程中申请内存空间
	LPVOID lpAddr = VirtualAllocEx(hProcess, NULL, m_strDllPath.GetLength() * 2+2, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (lpAddr == NULL)
	{
		MessageBox(L"在远程进程中申请内存失败");
		return;
	}
	
	//把Dll路径写入远程进程中
	if (FALSE == WriteProcessMemory(hProcess, lpAddr, m_strDllPath, m_strDllPath.GetLength() * 2 + 1, NULL))
	{
		MessageBox(L"在远程进程中写入数据失败");
		return;
	}
	
	//在远程进程中再开一个线程
	//Kernel32.dll  LoadLibraryW
	PTHREAD_START_ROUTINE pfnStartAssr=(PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "LoadLibraryW");
		
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnStartAssr, lpAddr, 0, NULL);
	if (hThread == NULL)
	{
		MessageBox(L"在远程进程中创建线程失败");
		return;
	}
	
	//等待远程线程结束
	WaitForSingleObject(hThread, INFINITE);
	//取Dll在目标进程的句柄
	DWORD remoteModule;
	GetExitCodeThread(hThread, &remoteModule);

	// 释放
	CloseHandle(hThread);
	VirtualFreeEx(hProcess, lpAddr, dllPathSize, MEM_DECOMMIT);

	MessageBox(L"注入成功", L"提示");
}

//菜单
void CInjectorDlg::OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码

	CMenu menu;
	menu.LoadMenu(IDR_MENU1);
	CMenu *pSubMenu=menu.GetSubMenu(0);

	POINT pos;
	GetCursorPos(&pos);
	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, this);




	*pResult = 0;
}


//浏览路径
void CInjectorDlg::OnBnClickedBtnChoice()
{
	//创建文件选择对话框
	//预设选择文件的扩展名
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Files (*.dll)|*.dll|All Files (*.*)|*.*||"), NULL);

	//弹出对话框   dlg.DoModal();
	//判断是否选择了
	if (IDCANCEL == dlg.DoModal())
		return;

	m_strDllPath = dlg.GetPathName();//获取选择中文件的路径
	UpdateData(FALSE);//把m_strDllPath显示出来

}


//关于
void CInjectorDlg::OnBnClickedBtnAbout()
{
	MessageBox(L"作者：洛兮\nQQ：1517807724\n仅供学习交流",L"关于");

}
