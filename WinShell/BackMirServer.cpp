// BackMirServer.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "../common/glog.h"

#include "Helper.h"
#include "BackMirServer.h"
#include "MainFrm.h"

#include "../CMainServer/CMainServer.h"
#include "serverdlg.h"
#include "runarg.h"
#include <direct.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#include <vld.h>
#endif


// CBackMirServerApp

BEGIN_MESSAGE_MAP(CBackMirServerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CBackMirServerApp::OnAppAbout)
END_MESSAGE_MAP()


// CBackMirServerApp ����

CBackMirServerApp::CBackMirServerApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CBackMirServerApp ����

CBackMirServerApp theApp;

int CreateGameDirs()
{
	char szRoot[MAX_PATH];
	GetRootPath(szRoot, MAX_PATH);

	char szDir[MAX_PATH];
	sprintf(szDir, "%s/conf", szRoot);

	mkdir(szDir);

	return 0;
}

// CBackMirServerApp ��ʼ��
static bool VerifyRunArg() {
	return true;
}

BOOL CBackMirServerApp::InitInstance()
{
#ifdef _DELAY_LOAD_DLL
	// set dll directory
	const char* pszAppPath = GetRootPath();
	char szDLLDir[MAX_PATH];
	strcpy(szDLLDir, pszAppPath);
#ifdef _DEBUG
	strcat_s(szDLLDir, "\\deps_d\\");
#else
	strcat_s(szDLLDir, "\\deps\\");
#endif
	if (TRUE != SetDllDirectory(szDLLDir))
	{
		::MessageBox(NULL, "�޷���ʼ��DLLģ��", "����", MB_ICONERROR | MB_OK);
		return FALSE;
	}
#endif
	CreateGameDirs();

	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// ��ʼ�� OLE ��
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	//	��ʼ����������
	if (!InitRunArg())
	{
		::MessageBox(NULL, "��ʼ����������ʧ��", "����", MB_ICONERROR);
		return FALSE;
	}
	CServerDlg dlg;
	dlg.DoModal();

	google::FlushLogFiles(google::GLOG_INFO);
	google::ShutdownGoogleLogging();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return TRUE;
}


// CBackMirServerApp ��Ϣ�������




// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
void CBackMirServerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CBackMirServerApp ��Ϣ�������

