#pragma once

#include <string>
using std::string;

// RegisterGameRoomDlg �Ի���

class RegisterGameRoomDlg : public CDialog
{
	DECLARE_DYNAMIC(RegisterGameRoomDlg)

public:
	RegisterGameRoomDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~RegisterGameRoomDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	CString m_xEditHostNameStr;
	CString m_xEditHostPortStr;
	CString m_xEditHostPasswordStr;
	CString m_xEditHostIPStr;
};
