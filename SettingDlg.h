#pragma once

//////////////////////////////////////////////////////////////////////////
#include <list>
//////////////////////////////////////////////////////////////////////////
extern CDialog* g_pSettingDlg;

// CSettingDlg �Ի���
struct OnlineUserInfo
{
	std::string xName;
	std::string xIP;
	std::string xMap;
	USHORT uPort;
	DWORD dwCnnIndex;
	DWORD dwDelay;
};

typedef std::list<OnlineUserInfo> OnlineUserInfoList;

class CSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSettingDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	virtual void OnCancel();
	virtual void OnOK();
	virtual void PostNcDestroy();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();

private:
	CListCtrl m_xListCtrl;
	CEdit m_xEdit;
	OnlineUserInfoList m_xOnlineUserInfoList;

public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
};
