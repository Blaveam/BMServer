// CpuQueryDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "BackMirServer.h"
#include "CpuQueryDlg.h"


// CCpuQueryDlg �Ի���

IMPLEMENT_DYNAMIC(CCpuQueryDlg, CDialog)

CCpuQueryDlg::CCpuQueryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCpuQueryDlg::IDD, pParent)
{
	m_bUseHTTech = false;
}

CCpuQueryDlg::~CCpuQueryDlg()
{
}

void CCpuQueryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCpuQueryDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CCpuQueryDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCpuQueryDlg ��Ϣ�������

void CCpuQueryDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bUseHTTech = true;
	OnOK();
}
