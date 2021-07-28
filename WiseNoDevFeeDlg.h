
// WiseNoDevFeeDlg.h: 头文件
//

#pragma once

#define WM_SHOWTASK     WM_USER + 100 //定义消息的值

// CWiseNoDevFeeDlg 对话框
class CWiseNoDevFeeDlg : public CDialogEx
{
// 构造
public:
	CWiseNoDevFeeDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WISENODEVFEE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	NOTIFYICONDATA m_nid;

// 实现
protected:
	HICON m_hIcon;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	LRESULT OnShowTask(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
//	afx_msg void OnCbnSelchangePools();
	CComboBox m_Pools;
	CEdit m_Wallet;
	CStatic m_SubmitWork;
	CEdit m_LogText;
	CButton m_StartButton;
	afx_msg void OnBnClickedStart();
//	afx_msg void OnEnChangeWallet();
	afx_msg void OnCbnSelchangePools();
	afx_msg void OnCbnSelendokPools();
	afx_msg void OnEnChangeWallet();
	afx_msg void OnBnClickedCanaccount();
	CButton m_CanAccount;
	CEdit m_Worker;
	afx_msg void OnEnChangeWorker();
	CStatic m_WalletLabel;
	afx_msg void OnEnChangeLogtext();
	CButton m_AutoRun;
	CButton m_AutoStart;
	afx_msg void OnBnClickedUpdate();
	afx_msg void OnBnClickedAutorun();
	afx_msg void OnBnClickedAutostart();
	CButton m_CheckPool;
	CEdit m_UserPool;
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCheckpool();
	afx_msg void OnEnChangeUserpool();
};
