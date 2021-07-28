
// WiseNoDevFeeDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "WiseNoDevFee.h"
#include "WiseNoDevFeeDlg.h"
#include "afxdialogex.h"
#include "json.hpp"
#include <atlstr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <string>
using namespace nlohmann;



CStringArray* SplitString(CString string, char pattern[])
{
	size_t nPattern = strlen(pattern);

	CStringArray* strArray = new CStringArray();
	CString strTemp;
	char c;
	BOOL bFind = FALSE;
	for (int i = 0; i < string.GetLength(); i++)
	{
		c = string[i];
		for (int j = 0; j < nPattern; j++)
		{
			if (c == pattern[j])
			{
				if (strTemp != "")
				{
					strArray->Add(strTemp);
				}
				bFind = TRUE;
				break;
			}
			bFind = FALSE;
		}
		if (bFind)
		{
			strTemp = "";
		}
		else
		{
			strTemp += c;
		}
	}
	if (strTemp != "")
	{
		strArray->Add(strTemp);
	}

	return strArray;
}
CString GetProfileValue(LPCTSTR section, LPCTSTR name, LPCTSTR defaultValue) {
	CString value;
	GetPrivateProfileString(section, name, defaultValue, value.GetBuffer(MAX_PATH), MAX_PATH, _T(".\\config.ini"));
	//value = AfxGetApp()->GetProfileString(section, name, defaultValue);
	return value;
}
void WriteProfileValue(LPCTSTR section, LPCTSTR name, LPCTSTR value) {
	WritePrivateProfileString(section, name, value, _T(".\\config.ini"));// 路径realse
	//AfxGetApp()->WriteProfileString(section, name, value);
}
// CWiseNoDevFeeDlg 对话框


BOOL CWiseNoDevFeeDlg::PreTranslateMessage(MSG* pMsg) {

	//屏蔽ESC关闭窗体/
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE;
	//屏蔽回车关闭窗体,但会导致回车在窗体上失效.
	//if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN && pMsg->wParam) return TRUE;
	//else
	return CDialog::PreTranslateMessage(pMsg);
}

CWiseNoDevFeeDlg::CWiseNoDevFeeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WISENODEVFEE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWiseNoDevFeeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_POOLS, m_Pools);
	DDX_Control(pDX, IDC_WALLET, m_Wallet);
	DDX_Control(pDX, IDC_SUBMITWORK, m_SubmitWork);
	DDX_Control(pDX, IDC_LOGTEXT, m_LogText);
	DDX_Control(pDX, IDC_START, m_StartButton);
	DDX_Control(pDX, IDC_CANACCOUNT, m_CanAccount);
	DDX_Control(pDX, IDC_WORKER, m_Worker);
	DDX_Control(pDX, IDC_WALLETLABEL, m_WalletLabel);
	DDX_Control(pDX, IDC_AUTORUN, m_AutoRun);
	DDX_Control(pDX, IDC_AUTOSTART, m_AutoStart);
	DDX_Control(pDX, IDC_CHECKPOOL, m_CheckPool);
	DDX_Control(pDX, IDC_USERPOOL, m_UserPool);
}

BEGIN_MESSAGE_MAP(CWiseNoDevFeeDlg, CDialogEx)
	ON_WM_PAINT()
	ON_MESSAGE(WM_SHOWTASK, OnShowTask)
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWiseNoDevFeeDlg::OnBnClickedOk)
//	ON_CBN_SELCHANGE(IDC_POOLS, &CWiseNoDevFeeDlg::OnCbnSelchangePools)
	ON_BN_CLICKED(IDC_START, &CWiseNoDevFeeDlg::OnBnClickedStart)
//	ON_EN_CHANGE(IDC_WALLET, &CWiseNoDevFeeDlg::OnEnChangeWallet)
ON_CBN_SELCHANGE(IDC_POOLS, &CWiseNoDevFeeDlg::OnCbnSelchangePools)
ON_CBN_SELENDOK(IDC_POOLS, &CWiseNoDevFeeDlg::OnCbnSelendokPools)
ON_EN_CHANGE(IDC_WALLET, &CWiseNoDevFeeDlg::OnEnChangeWallet)
ON_BN_CLICKED(IDC_CANACCOUNT, &CWiseNoDevFeeDlg::OnBnClickedCanaccount)
ON_EN_CHANGE(IDC_WORKER, &CWiseNoDevFeeDlg::OnEnChangeWorker)
ON_EN_CHANGE(IDC_LOGTEXT, &CWiseNoDevFeeDlg::OnEnChangeLogtext)
ON_BN_CLICKED(IDC_UPDATE, &CWiseNoDevFeeDlg::OnBnClickedUpdate)
ON_BN_CLICKED(IDC_AUTORUN, &CWiseNoDevFeeDlg::OnBnClickedAutorun)
ON_BN_CLICKED(IDC_AUTOSTART, &CWiseNoDevFeeDlg::OnBnClickedAutostart)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_CHECKPOOL, &CWiseNoDevFeeDlg::OnBnClickedCheckpool)
ON_EN_CHANGE(IDC_USERPOOL, &CWiseNoDevFeeDlg::OnEnChangeUserpool)
END_MESSAGE_MAP()

void SetAutoRun(boolean autoStart) {
	HKEY hKey;
	//找到系统的启动项   
	CString lpRun = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	//打开启动项Key   
	long lRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_ALL_ACCESS, &hKey);
	if (lRet == ERROR_SUCCESS) {
		if (autoStart) {
			TCHAR pFileName[MAX_PATH] = { 0 };
			//得到程序自身的全路径   
			DWORD dwRet = GetModuleFileName(NULL, pFileName, MAX_PATH);
			//添加一个子Key,并设置值, 下面应用程序名字（不加后缀.exe）  
			lRet = RegSetValueEx(hKey, _T("WiseNoDevFee"), 0, REG_SZ, (LPBYTE)pFileName, (lstrlen(pFileName) + 1) * sizeof(TCHAR));
		}
		else {
			RegDeleteValue(hKey, _T("WiseNoDevFee"));
		}
		RegDeleteValue(hKey, _T("NTMinerNoDevFee"));
		//关闭注册表   
		RegCloseKey(hKey);
	}
}
// CWiseNoDevFeeDlg 消息处理程序

BOOL CWiseNoDevFeeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	std::string content;
	int result = GetFileContent("https://github.com/MinerProxy/WiseNoDevFee/blob/main/config/pools.json", &content);
	if (result == 0) {
		auto lists = json::parse(content);
		for (auto& el : lists.items()) {
			m_Pools.AddString((LPCTSTR)&el.value());
		}
	}
	m_Pools.SetCurSel(0);
	
	CString value = GetProfileValue(_T("Settings"), _T("Pool"), _T(""));
	int nIndex = m_Pools.SelectString(0, value);
	if (nIndex == -1) {
		m_Pools.SetCurSel(0);
	}

	value = GetProfileValue(_T("Settings"), _T("UserPool"), _T("eth-pool.beepool.org:9530"));
	m_UserPool.SetWindowTextW(value);

	value = GetProfileValue(_T("Settings"), _T("CheckPool"), _T("false"));
	if (value.Compare(_T("true")) == 0) {
		m_Pools.ShowWindow(SW_HIDE);
		m_UserPool.ShowWindow(SW_SHOW);
		m_CheckPool.SetCheck(1);
	}
	else {
		m_Pools.ShowWindow(SW_SHOW);
		m_UserPool.ShowWindow(SW_HIDE);
		m_CheckPool.SetCheck(0);
	}
	
	value = GetProfileValue(_T("Settings"), _T("Wallet"), _T(""));
	m_Wallet.SetWindowTextW(value);

	value = GetProfileValue(_T("Settings"), _T("Worker"), _T("NoDev"));
	m_Worker.SetWindowTextW(value);

	CString checked = GetProfileValue(_T("Settings"), _T("CanAccount"), _T("false"));
	if (checked.Compare(_T("true")) == 0) {
		m_CanAccount.SetCheck(1);
		m_WalletLabel.SetWindowTextW(_T("账号名称："));
		//m_Worker.EnableWindow(true);
	}
	else {
		m_CanAccount.SetCheck(0);
		m_WalletLabel.SetWindowTextW(_T("钱包地址："));
		//m_Worker.EnableWindow(false);
	}
	m_AutoRun.EnableToolTips();

	checked = GetProfileValue(_T("Settings"), _T("AutoRun"), _T("true"));
	if (checked.Compare(_T("true")) == 0) {
		m_AutoRun.SetCheck(1);
		SetAutoRun(true);
	}
	else {
		m_AutoRun.SetCheck(0);
		SetAutoRun(false);
	}

	checked = GetProfileValue(_T("Settings"), _T("AutoStart"), _T("false"));
	boolean autoStart = (checked.Compare(_T("true")) == 0);
	if (autoStart) {
		m_AutoStart.SetCheck(1);
	}
	else {
		m_AutoStart.SetCheck(0);
	}

	m_LogText.SetReadOnly();
	m_LogText.SetWindowTextW(_T("内核反抽水 V0.8 2021-07-28\r\n自动将内核抽水返还到您的钱包地址，目前支持 GMiner，Phoenix Miner，ClaymoreMore。\r\n仅供内部参考学习使用，勿要用于盈利或者其他用途！\r\n"));

	if (autoStart) {
		OnBnClickedStart();
	}

	//---------------------------托盘显示---------------------------------//

	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = this->m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK; // 自定义的消息名称
	m_nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	wcscpy_s(m_nid.szTip, _T("WiseNoDevFee反抽水工具"));
	//strcpy(m_nid.szTip, "服务器程序"); // 信息提示条为"服务器程序"，VS2008 UNICODE编码用wcscpy_s()函数
	Shell_NotifyIcon(NIM_ADD, &m_nid);// 在托盘区添加图标


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWiseNoDevFeeDlg::OnPaint()
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
HCURSOR CWiseNoDevFeeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWiseNoDevFeeDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


//void CWiseNoDevFeeDlg::OnCbnSelchangePools()
//{
//	// TODO: 在此添加控件通知处理程序代码
//}


void CWiseNoDevFeeDlg::OnBnClickedStart()
{
	CString title;
	m_StartButton.GetWindowTextW(title);
	if (title.Compare(_T("正在反抽水")) == 0) {
		m_StartButton.SetWindowTextW(_T("开始"));
		logText(_T("反抽水已经停止。"));
		stopNoDevFee();
		AfxMessageBox(_T("WiseNoDevFee已经成功停止，如要继续，请按开始按钮！"));
	}
	else {
		CString host, port, wallet, worker;
		CString pool;
		CStringArray* pools;
		if (m_CheckPool.GetCheck() == 1) {
			m_UserPool.GetWindowTextW(pool);
			if (pool.IsEmpty() || pool.Find(_T(":")) == -1) {
				AfxMessageBox(_T("请填写矿池地址：主机名:端口号！"));
				return;
			}
			pools = SplitString(pool, ":");
		}
		else {
			m_UserPool.GetWindowTextW(pool);
			m_Pools.GetWindowTextW(pool);
			if (pool.IsEmpty()) {
				AfxMessageBox(_T("请选择矿池地址！"));
			}
			pools = SplitString(pool, " ");
			pools = SplitString(pools->GetAt(1), ":");
		}
		host = pools->GetAt(0);
		port = pools->GetAt(1);

		m_Worker.GetWindowTextW(worker);
		m_Wallet.GetWindowTextW(wallet);
		if (wallet.IsEmpty()) {
			AfxMessageBox(_T("请填写钱包地址！"));
			return;
		}
		int checked = m_CanAccount.GetCheck();
		if (checked != 1 && wallet.GetLength() != 42) {
			AfxMessageBox(_T("钱包地址是以0x开头的42位字符！\r\n如果需要使用账号方式，请勾选旁边的账号方式"));
			return;
		}
		if (checked == 1 && (wallet.GetLength() + worker.GetLength() > 42)) {
			AfxMessageBox(_T("账号方式不能填写钱包地址，且长度不能超过42！"));
			return;
		}

		int result = startNoDevFee(CW2A(host), atoi(CW2A(port)), CW2A(worker), CW2A(wallet));
		if (result == 0) {
			m_StartButton.SetWindowTextW(_T("正在反抽水"));
			//CString c;
			//c.Format(_T("矿池地址：%s:%d"), host, port);
			logText(_T("已经成功启动（请等待出现 Found DevFee.. 即表示成功拦截）！"));
			logText(_T("如长久未出现 Found DevFee，请重启内核！"));
		}
		else if (result == 1) {
			logText(_T("未能找到矿池地址的IP地址，请检查网络！"));
		}
		else if (result == 2) {
			logText(_T("已经启动，无需重复启动。"));
		}
		else {
			logText(_T("未知错误。"));
		}
		
	}
	
	
	// TODO: 在此添加控件通知处理程序代码
}


//void CWiseNoDevFeeDlg::OnEnChangeWallet()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//}


void CWiseNoDevFeeDlg::OnCbnSelchangePools()
{
	CString text;
	m_Pools.GetLBText(m_Pools.GetCurSel(), text);
	if (!text.IsEmpty()) {
		WriteProfileValue(_T("Settings"), _T("Pool"), text);
	}
}


void CWiseNoDevFeeDlg::OnCbnSelendokPools()
{

}


void CWiseNoDevFeeDlg::OnEnChangeWallet()
{
	CString text;
	m_Wallet.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		WriteProfileValue(_T("Settings"), _T("Wallet"), text);
	}
}


void CWiseNoDevFeeDlg::OnBnClickedCanaccount()
{
	int checked = m_CanAccount.GetCheck();
	WriteProfileValue(_T("Settings"), _T("CanAccount"), checked == 1 ? _T("true") : _T("false"));
	if (checked == 1) {
		//m_Worker.EnableWindow(true);
		m_WalletLabel.SetWindowTextW(_T("账号名称："));
	}
	else {
		m_WalletLabel.SetWindowTextW(_T("钱包地址："));
		//m_Worker.EnableWindow(false);
	}
	CString worker;
	m_Worker.GetWindowTextW(worker);
	if (worker.GetLength() > 6) {
		AfxMessageBox(_T("钱包地址方式，矿机名称不能超过6个字符，请更改矿机名。"));
		m_Worker.SetWindowTextW(_T("NoDev"));
	}
}


void CWiseNoDevFeeDlg::OnEnChangeWorker()
{
	CString text;
	m_Worker.GetWindowTextW(text);
	if (!text.IsEmpty()) {
		WriteProfileValue(_T("Settings"), _T("Worker"), text);
	}
}


void CWiseNoDevFeeDlg::OnEnChangeLogtext()
{
	
}


void CWiseNoDevFeeDlg::OnBnClickedUpdate()
{
	ShellExecute(0, NULL, _T("https://github.com/MinerProxy/WiseNoDevFee/releases"), NULL, NULL, SW_NORMAL);
}


void CWiseNoDevFeeDlg::OnBnClickedAutorun()
{
	int checked = m_AutoRun.GetCheck();
	HKEY hKey;
	//找到系统的启动项   
	CString lpRun = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	//打开启动项Key   
	long lRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_ALL_ACCESS, &hKey);
	if (lRet == ERROR_SUCCESS) {
		if (checked == 1) {
			TCHAR pFileName[MAX_PATH] = { 0 };
			//得到程序自身的全路径   
			DWORD dwRet = GetModuleFileName(NULL, pFileName, MAX_PATH);
			//添加一个子Key,并设置值, 下面应用程序名字（不加后缀.exe）  
			lRet = RegSetValueEx(hKey, _T("WiseNoDevFee"), 0, REG_SZ, (LPBYTE)pFileName, (lstrlen(pFileName) + 1) * sizeof(TCHAR));
		}
		else {
			RegDeleteValue(hKey, _T("WiseNoDevFee"));
		}
		//关闭注册表   
		RegCloseKey(hKey);
	}

	WriteProfileValue(_T("Settings"), _T("AutoRun"), checked == 1 ? _T("true") : _T("false"));
}


void CWiseNoDevFeeDlg::OnBnClickedAutostart()
{
	int checked = m_AutoStart.GetCheck();
	WriteProfileValue(_T("Settings"), _T("AutoStart"), checked == 1 ? _T("true") : _T("false"));
}


void CWiseNoDevFeeDlg::OnClose()
{
	ShowWindow(SW_HIDE);
	//CDialogEx::OnClose();
}
LRESULT CWiseNoDevFeeDlg::OnShowTask(WPARAM wParam, LPARAM lParam)
{
	if (wParam != IDR_MAINFRAME)
		return 1;
	switch (lParam)
	{
	case WM_RBUTTONUP: // 右键起来时弹出菜单
	{
		LPPOINT lpoint = new tagPOINT;
		::GetCursorPos(lpoint);// 得到鼠标位置
		CMenu menu;
		menu.CreatePopupMenu();// 声明一个弹出式菜单
		menu.AppendMenuW(MF_STRING, WM_DESTROY, _T("退出WiseNoDevFee"));
		menu.TrackPopupMenu(TPM_LEFTALIGN, lpoint->x, lpoint->y, this);
		HMENU hmenu = menu.Detach();
		menu.DestroyMenu();
		delete lpoint;
	}
	break;
	case WM_LBUTTONDOWN: // 双击左键的处理
	{

		if (this->IsWindowVisible()) //判断窗口当前状态
		{
			this->ShowWindow(SW_HIDE); //隐藏窗口
		}
		else
		{
			this->ShowWindow(SW_SHOWNA); //显示窗口
			this->SetForegroundWindow();
		}
	}

	break;
	}
	return 0;
}

void CWiseNoDevFeeDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	// 在托盘区删除图标
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}


void CWiseNoDevFeeDlg::OnBnClickedCheckpool()
{
	int checked = m_CheckPool.GetCheck();
	WriteProfileValue(_T("Settings"), _T("CheckPool"), checked == 1 ? _T("true") : _T("false"));
	if (checked == 1) {
		m_Pools.ShowWindow(SW_HIDE);
		m_UserPool.ShowWindow(SW_SHOW);
	}
	else {
		m_Pools.ShowWindow(SW_SHOW);
		m_UserPool.ShowWindow(SW_HIDE);
	}
}

void CWiseNoDevFeeDlg::OnEnChangeUserpool()
{
	CString text;
	m_UserPool.GetWindowTextW(text);
	WriteProfileValue(_T("Settings"), _T("UserPool"), text);
}