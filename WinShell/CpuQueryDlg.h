#pragma once


// CCpuQueryDlg �Ի���

class CCpuQueryDlg : public CDialog
{
	DECLARE_DYNAMIC(CCpuQueryDlg)

public:
	CCpuQueryDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCpuQueryDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG5 };

	inline bool GetUseHTTech()
	{
		return m_bUseHTTech;
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	bool m_bUseHTTech;
public:
	afx_msg void OnBnClickedOk();
};
