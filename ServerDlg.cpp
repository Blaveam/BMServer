// ServerDlg.cpp : ʵ���ļ�
//
#include "stdafx.h"
#include "BackMirServer.h"
#include "ServerDlg.h"
#include "RegisterGameRoomDlg.h"
#include "IOServer/SServerEngine.h"
#include "CMainServer/CMainServer.h"
#include "Helper.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <Shlwapi.h>
#include "./GameWorld/struct.h"
#include "./GameWorld/GameWorld.h"
#include "./GameWorld/ObjectEngine.h"
#include "./GameWorld/ExceptionHandler.h"
#include "SettingDlg.h"
#include "../CommonModule/CommandLineHelper.h"
#include "../CommonModule/SimpleIni.h"
#include "../CommonModule/BMHttpManager.h"
#include "../CommonModule/cJSON.h"
#include "ConfigDlg.h"
#include "runarg.h"
#include "../CommonModule/version.h"

using std::string;
//////////////////////////////////////////////////////////////////////////
void SetRandomTitle(HWND _hWnd)
{
	if (CMainServer::GetInstance()->GetServerMode() == GM_LOGIN) {
		char szTitle[250];
		szTitle[0] = 0;
		sprintf(szTitle, "ServerID[%d] ServerName[%s]", GetServerID(), GetRunArg("servername"));
		SetWindowText(_hWnd, szTitle);
		return;
	}
	char str[]="ABCDEFGHIJKLMHOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	char szCaption[256]={0};
	INT i,leg;
	srand((unsigned)time(NULL));	//��ÿ�β������������ͬ

	for(i=0;i<rand()%4+10;i++){
		//���ⳤ����rand()%4+10����,����Ϊ10��11��12��13��14
		leg = rand()%strlen(str);
		szCaption[i]=str[leg];	//�����⸳ֵ
	}

	SetWindowText(_hWnd, szCaption);
}

string GBKToUTF8(const std::string& strGBK)  
{  
	string strOutUTF8 = "";  
	WCHAR * str1;  
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);  
	str1 = new WCHAR[n];  
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);  
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);  
	char * str2 = new char[n];  
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);  
	strOutUTF8 = str2;  
	delete[]str1;  
	str1 = NULL;  
	delete[]str2;  
	str2 = NULL;  
	return strOutUTF8;  
}  
//////////////////////////////////////////////////////////////////////////
HWND g_hServerDlg = NULL;
HeroObjectList g_xWaitDeleteHeros;
// CServerDlg �Ի���
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CServerDlg, CDialog)

CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerDlg::IDD, pParent)
{
	//	Generate the root path
	GetRootPath();
	m_pxMainServer = CMainServer::GetInstance();
	m_pxMainServer->SetServerShell(this);
	g_hServerDlg = GetSafeHwnd();
	m_hListBkBrush = ::CreateSolidBrush(RGB(0, 0, 0));
	m_bInitNetwork = FALSE;
	m_bVersionOK = FALSE;
	m_nGameRoomServerID = 0;
	m_nOnlinePlayerCount = 0;
	m_dwServerStartTime = 0;
}

CServerDlg::~CServerDlg()
{
	::DeleteObject(m_hListBkBrush);
	if(NULL != m_pxMainServer)
	{
		//delete m_pxMainServer;
		//m_pxMainServer = NULL;
	}
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


//////////////////////////////////////////////////////////////////////////
BOOL CServerDlg::OnInitDialog()
{
#ifdef _DEBUG
	//AfxMessageBox("DEBUG");
#endif
	g_xConsole.InitConsole();

	g_hServerDlg = GetSafeHwnd();
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	m_bInitNetwork = m_pxMainServer->InitNetWork() ? TRUE : FALSE;
	if(m_bInitNetwork)
	{
		AddTextToMessageBoardFmt("��ʼ���������ɹ�");
		if(m_pxMainServer->InitDatabase())
		{
			m_bVersionOK = TRUE;
			AutoRun();
		}
		else
		{
			AddTextToMessageBoardFmt("��ʼ��������ʧ��");
			LOG(ERROR) << "��ʼ��������ʧ��";
		}
	}
	else
	{
		AddTextToMessageBoardFmt("��ʼ��������ʧ��");
		LOG(FATAL) << "��ʼ��������ʧ��";
	}

	//	�����쳣����
	SetUnhandledExceptionFilter(&BM_UnhandledExceptionFilter);
	//	��ʱɾ���ȴ�ɾ�������
	SetTimer(TIMER_DELETEPLAYERS, 5000, NULL);
	// Initialize message board timer check
	SetTimer(TIMER_MSGBOARD, 50, NULL);

	//	����CRC��Ϣ
	if(!m_pxMainServer->InitCRCThread())
	{

	}

	SetRandomTitle(GetSafeHwnd());

#ifndef _DEBUG
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
#endif

	return __super::OnInitDialog();
}


//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CServerDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CServerDlg::OnBnStartClicked)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_HELPER_ADDINFO, &CServerDlg::OnAddInformation)
	ON_MESSAGE(WM_UPDATE_INFO, &CServerDlg::OnUpdateDialogInfo)
	ON_BN_CLICKED(IDC_BUTTON2, &CServerDlg::OnBnStopClicked)
	ON_BN_CLICKED(IDC_BUTTON3, &CServerDlg::OnBnClickedButton3)
	ON_MESSAGE(WM_INSERTMAPKEY, &CServerDlg::OnUserMessage)
	ON_MESSAGE(WM_CLOSECONNECTION, &CServerDlg::OnCloseConnection)
	ON_MESSAGE(WM_REMOVEPLAYER, &CServerDlg::OnRemoveHeroObject)
	ON_MESSAGE(WM_PLAYERCOUNT, &CServerDlg::OnUpdatePlayerCount)
	ON_MESSAGE(WM_DISTINCTIP, &CServerDlg::OnUpdateDistinctIP)
	ON_BN_CLICKED(IDC_BUTTON4, &CServerDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CServerDlg::OnBnClickedButton5)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON6, &CServerDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CServerDlg::OnBnClickedButton7)
END_MESSAGE_MAP()


// CServerDlg ��Ϣ�������

/************************************************************************/
/* ��������
/************************************************************************/
int CServerDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//lpCreateStruct->style |= WS_OVERLAPPEDWINDOW;
	return  __super::OnCreate(lpCreateStruct);
}

void CServerDlg::OnDestroy()
{
	if(0 != m_nGameRoomServerID)
	{
		char szUrl[MAX_PATH] = {0};
		sprintf(szUrl, "%s/removegs?id=%d", m_xRegisterGameRoomUrl.c_str(), m_nGameRoomServerID);
		BMHttpManager::GetInstance()->DoGetRequestSync(szUrl, fastdelegate::bind(&CServerDlg::OnRemoveGsResult, this));
	}

	__super::OnDestroy();
}

/************************************************************************/
/* ��������
/************************************************************************/
void CServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CListBox* pxList = static_cast<CListBox*>(GetDlgItem(IDC_LIST1));
	CHECK(pxList != NULL);
	//pxList->MoveWindow(10, 10, cx - 20, cy - 10 - 50);
}

/************************************************************************/
/* ����ListBox������ɫ
/************************************************************************/
HBRUSH CServerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch(nCtlColor)
	{
	case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(RGB(0, 255, 0));
			pDC->SetBkMode(TRANSPARENT);
			return m_hListBkBrush;
		}break;
	default:
		{
			return __super::OnCtlColor(pDC, pWnd, nCtlColor);
		}break;
	}
}

/************************************************************************/
/* ����������
/************************************************************************/
void CServerDlg::OnBnStartClicked()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(!m_bInitNetwork)
	{
		AddTextToMessageBoardFmt("�޷�����������");
		return;
	}

	if(!m_bVersionOK)
	{
		AddTextToMessageBoardFmt("�޷�����������");
		return;
	}

	const char* szCfgFile = RunArgGetConfigFile();

	if(!PathFileExists(szCfgFile))
	{
		AddTextToMessageBoardFmt("�޷���ȡ������������Ϣ (%s)", szCfgFile);
		LOG(INFO) << "�޷���ȡ������������Ϣ:" << szCfgFile;
		return;
	}

	//	��ȡ����
	char szValue[50];
	::GetPrivateProfileString("SERVER", "IP", "", szValue, sizeof(szValue), szCfgFile);
	if(strlen(szValue) == 0)
	{
		LOG(WARNING) << "�����ļ�IP��ȡֵΪ��";
		return;
	}
	WORD wPort = 0;
	wPort = ::GetPrivateProfileInt("SERVER", "PORT", 0, szCfgFile);
	if(wPort == 0)
	{
		LOG(WARNING) << "�����ļ�PORT��ȡֵΪ��";
		return;
	}

	if(m_pxMainServer->StartServer(szValue, wPort))
	{
		AddTextToMessageBoardFmt("�����������ɹ�����ʼ����");
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
		char szIpPort[100];
		sprintf(szIpPort, "������IP:%s �˿�:%d",
			szValue, wPort);
		GetDlgItem(IDC_IPPORT)->SetWindowText(szIpPort);
		m_dwServerStartTime = GetTickCount();
		SetTimer(TIMER_UPDATERUNNINGTIME, 1 * 1000, NULL);
	}
	else
	{
		AddTextToMessageBoardFmt("����������ʧ��");
		LOG(ERROR) << "����������ʧ�� IP[" << szValue << "] PORT[" << wPort << "]";
	}
}

/************************************************************************/
/* ���Listbox��Ϣ
/************************************************************************/
LRESULT CServerDlg::OnAddInformation(WPARAM wParam, LPARAM lParam)
{
	CListBox* pxList = static_cast<CListBox*>(GetDlgItem(IDC_LIST1));
	CHECK(pxList != NULL);

	int nIndex = pxList->GetCount();
	if(nIndex == LB_ERR)
	{
		return S_FALSE;
	}

	if(nIndex > 50)
	{
		pxList->ResetContent();
	}

	const char* pText = (const char*)wParam;
	pxList->InsertString(-1, pText);
	pxList->SetTopIndex(pxList->GetCount() - 1);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CServerDlg::AutoRun()
{
	//CommandLineHelper xHelper;
	//if(xHelper.InitParam())
	{
		const char* pszValue = GetRunArg("loginsvr");
		if(pszValue != NULL)
		{
			CMainServer::GetInstance()->SetServerMode(GM_LOGIN);
			string xLoginAddr = pszValue;
			CMainServer::GetInstance()->SetLoginAddr(xLoginAddr);
		}

		pszValue = GetRunArg("listenip");
		if(pszValue != NULL)
		{
			char szIP[16];
			char szBuf[16];
			szIP[15] = 0;
			WORD wPort = 0;
			int nPortPos = 0;
			for(int i = 0; i < 16; ++i)
			{
				if(pszValue[i] == ':')
				{
					nPortPos = i;
					szBuf[i] = 0;
					break;
				}
				else
				{
					szBuf[i] = pszValue[i];
				}
			}
			strcpy(szIP, szBuf);
			++nPortPos;

			for(int i = nPortPos; ; ++i)
			{
				if(pszValue[i] == 0)
				{
					szBuf[i - nPortPos] = 0;
					break;
				}
				szBuf[i - nPortPos] = pszValue[i];
			}
			wPort = (WORD)atoi(szBuf);

			LOG(INFO) << "IP:" << szIP << ", Port:" << wPort;

			if (m_pxMainServer->StartServer(szIP, wPort))
			{
				AddTextToMessageBoardFmt("�����������ɹ�����ʼ����");
				GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
				GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
				char szIpPort[100];
				sprintf(szIpPort, "������IP:%s �˿�:%d",
					szIP, wPort);
				GetDlgItem(IDC_IPPORT)->SetWindowText(szIpPort);
				m_dwServerStartTime = GetTickCount();
				SetTimer(TIMER_UPDATERUNNINGTIME, 1 * 1000, NULL);

				ShowWindow(SW_HIDE);
			}
			else
			{
				AddTextToMessageBoardFmt("����������ʧ��");
				LOG(ERROR) << "����������ʧ�� IP[" << szIP << "] PORT[" << wPort << "]";
			}
		}
	}

	return;  
}
/************************************************************************/
/* ������ʾ����
/************************************************************************/
LRESULT CServerDlg::OnUpdateDialogInfo(WPARAM wParam, LPARAM lParam)
{
	extern const char* g_szMode[2];

	ServerState* pState = (ServerState*)wParam;
	char szOutput[MAX_PATH];
	/*sprintf(szOutput, "��������: %d",
		pState->wOnline);
	GetDlgItem(IDC_USERSUM)->SetWindowText(szOutput);*/

	sprintf(szOutput, "����״̬: %s",
		g_szMode[pState->bMode]);
	GetDlgItem(IDC_MODE)->SetWindowText(szOutput);

	return S_OK;
}
void CServerDlg::OnBnStopClicked()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//m_bInitNetwork = false;
	//m_pxMainServer->StopServer();
	//bool bIsPaused = GameWorld
	//GameWorld::GetInstance().Pause();

	//GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	//GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
}

LRESULT CServerDlg::OnRemoveHeroObject(WPARAM wParam, LPARAM lParam)
{
	m_pxMainServer->DecOnlineUsers();
	m_pxMainServer->UpdateServerState();
	g_xWaitDeleteHeros.push_back((HeroObject*)lParam);
	return S_OK;
}

LRESULT CServerDlg::OnUpdatePlayerCount(WPARAM wParam, LPARAM lParam)
{
	char szOutput[MAX_PATH];
	sprintf(szOutput, "��������: %d",
		wParam);
	m_nOnlinePlayerCount = wParam;
	GetDlgItem(IDC_USERSUM)->SetWindowText(szOutput);

	sprintf(szOutput, "��������߼�: %d",
		lParam);
	GetDlgItem(IDC_SVRINFO)->SetWindowText(szOutput);

	return S_OK;
}

LRESULT CServerDlg::OnUpdateDistinctIP(WPARAM wParam, LPARAM lParam)
{
	char szOutput[MAX_PATH];
	sprintf(szOutput, "����IP��: %d",
		wParam);
	m_nOnlinePlayerCount = wParam;
	GetDlgItem(IDC_DISTINCTIP)->SetWindowText(szOutput);

	return S_OK;
}

void CServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == TIMER_DELETEPLAYERS)
	{
		if(!g_xWaitDeleteHeros.empty())
		{
			HeroObjectList::iterator begIter = g_xWaitDeleteHeros.begin();
			HeroObject* pHero = NULL;

			for(begIter;
				begIter != g_xWaitDeleteHeros.end();
				)
			{
				pHero = *begIter;
				bool bErased = false;

				if(pHero->GetUserIndex() > 0 &&
					pHero->GetUserIndex() <= MAX_USER_NUMBER)
				{
					if(g_pxHeros[pHero->GetUserIndex()] != pHero)
					{
						LOG(INFO) << "Delete a hang up hero object...";
						delete pHero;
						bErased = true;
					}
				}

				if(bErased)
				{
					begIter = g_xWaitDeleteHeros.erase(begIter);
				}
				else
				{
					++begIter;
				}
			}
		}
	}
	else if(nIDEvent == TIMER_REGISTERGS)
	{
		RegisterGameRoom();
	}
	else if(nIDEvent == TIMER_UPDATERUNNINGTIME)
	{
		DWORD dwMs = GetTickCount() - m_dwServerStartTime;
		int nSec = dwMs / 1000;

		char szMsg[50] = {0};

		CWnd* pLabel = GetDlgItem(IDC_RUNNINGTIME);

		if(nSec < 60)
		{
			sprintf(szMsg, "����ʱ��: %d��", nSec);
		}
		else if(nSec < 60 * 60)
		{
			int nMin = nSec / 60;
			int nSecLeft = nSec - nMin * 60;
			sprintf(szMsg, "����ʱ��: %d�� %d��", nMin, nSecLeft);
		}
		else
		{
			int nHour = nSec / 60 / 60;
			int nMin = (nSec - nHour * 60 * 60) / 60;
			int nSecLeft = nSec - nHour * 3600 - nMin * 60;
			sprintf(szMsg, "����ʱ��: %dСʱ %d��", nHour, nMin);
		}
		pLabel->SetWindowText(szMsg);
	}
	else if (nIDEvent == TIMER_MSGBOARD) {
		m_xMsgBoardMu.lock();
		// Apply message board texts
		for (auto &text : m_xMsgBoardTextList) {
			AddTextToMessageBoard(text.c_str());
		}

		m_xMsgBoardTextList.clear();
		m_xMsgBoardMu.unlock();
	}
}

void CServerDlg::AddTextToMessageBoard(const char* fmt) {
	CListBox* pxList = static_cast<CListBox*>(GetDlgItem(IDC_LIST1));
	CHECK(pxList != NULL);

	int nIndex = pxList->GetCount();
	if (nIndex == LB_ERR)
	{
		return;
	}

	if (nIndex > 50)
	{
		pxList->ResetContent();
	}

	pxList->InsertString(-1, fmt);
	pxList->SetTopIndex(pxList->GetCount() - 1);
}

void CServerDlg::AddTextToMessageBoardFmt(const char* fmt, ...) {
	char buffer[MAX_PATH];

	va_list args;
	va_start(args, fmt);
	_vsnprintf(buffer, sizeof(buffer), fmt, args);

	SYSTEMTIME lpTime;
	GetLocalTime(&lpTime);

	char logTime[MAX_PATH];
	sprintf(logTime, "[%d-%d-%d %02d:%02d:%02d] ", lpTime.wYear, lpTime.wMonth, lpTime.wDay, lpTime.wHour, lpTime.wMinute, lpTime.wSecond);
	strcat(logTime, buffer);

	AddTextToMessageBoard(logTime);
}

void CServerDlg::OnBnClickedButton3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	google::FlushLogFiles(google::GLOG_INFO);
}

LRESULT CServerDlg::OnUserMessage(WPARAM _wParam, LPARAM _lParam)
{
	return 1;
}

LRESULT CServerDlg::OnCloseConnection(WPARAM _wParam, LPARAM _lParam)
{
	LOG(INFO) << "Force close connection[" << _wParam << "]";
	m_pxMainServer->GetIOServer()->CloseUserConnection(_wParam);
	return 1;
}

void CServerDlg::OnBnClickedButton4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
#ifdef _DEBUG
	DelayedProcess dp;
	dp.uOp = DP_RELOADSCRIPT;
	GameWorld::GetInstance().AddDelayedProcess(&dp);
#endif
}


void CServerDlg::OnBnClickedButton5()
{
	RECORD_FUNCNAME_UI;
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//	������Ϸ
	if(g_pSettingDlg == NULL)
	{
		g_pSettingDlg = new CSettingDlg(this);
		g_pSettingDlg->Create(IDD_DIALOG2, this);
		g_pSettingDlg->ShowWindow(SW_SHOW);
	}
}


void CServerDlg::OnClose()
{
	GameWorld* pWorld = GameWorld::GetInstancePtr();

	if(pWorld->GetWorldState() == WS_WORKING)
	{
		int nSelect = AfxMessageBox("�����������У�ȷ��Ҫ�رշ�������", MB_YESNO);
		if(IDYES == nSelect)
		{
			//	��ע����Ϸ������
			if(0 != m_nGameRoomServerID)
			{
				KillTimer(TIMER_REGISTERGS);
			}

			//	�ȴ�������ֹͣ
			/*pWorld->Terminate(0);
			while(pWorld->GetWorldState() != WS_STOP)
			{
				Sleep(1);
			}*/

			// �ȴ���������ֹͣ
			CMainServer::GetInstance()->WaitForStopEngine();
			
			__super::OnClose();
		}
		else
		{
			return;
		}
	}
	else
	{
		__super::OnClose();
	}
}

void CServerDlg::OnOK()
{
	//	nothing
}

void CServerDlg::OnCancel()
{
	__super::OnCancel();
}

BOOL CServerDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam)
		{
		case VK_ESCAPE:
			{
				GameWorld* pWorld = GameWorld::GetInstancePtr();

				if(pWorld->GetWorldState() == WS_WORKING)
				{
					int nSelect = AfxMessageBox("�����������У�ȷ��Ҫ�رշ�������", MB_YESNO);
					if(IDYES == nSelect)
					{
						//	��ע����Ϸ������
						if(0 != m_nGameRoomServerID)
						{
							KillTimer(TIMER_REGISTERGS);
						}

						//	�ȴ�������ֹͣ
						pWorld->Terminate(0);
						while(pWorld->GetWorldState() != WS_STOP)
						{
							Sleep(1);
						}
					}
					else
					{
						return TRUE;
					}
				}
			}break;
		}
	}  

	return CDialog::PreTranslateMessage(pMsg);  
}

void CServerDlg::OnBnClickedButton6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(GameWorld::GetInstancePtr()->GetWorldState() != WS_WORKING)
	{
		AfxMessageBox("��������δ�������޷�ע������������", MB_OK | MB_ICONERROR);
		return;
	}

	bool bIpValid = true;
	string& refListenIP = m_pxMainServer->GetListenIP();
	if(refListenIP.size() < 7)
	{
		bIpValid = false;
	}
	else
	{
		if(refListenIP[0] == '1' &&
			refListenIP[1] == '2' &&
			refListenIP[2] == '7')
		{
			bIpValid = false;
		}
	}
	if(!bIpValid)
	{
		AfxMessageBox("��ǰΪ����ģʽ���޷�ע�ᵽ����������", MB_OK | MB_ICONERROR);
#ifdef _DEBUG
#else
		return;
#endif
	}

	//	��ȡ�����url
	char szDestPath[MAX_PATH] = {0};
	strcpy(szDestPath, GetRootPath());
	strcat(szDestPath, "/config/url_2.ini");

	if(!PathFileExists(szDestPath))
	{
		AfxMessageBox("�޷���ȡ�����ַ", MB_OK | MB_ICONERROR);
		return;
	}

	CSimpleIniA xIniFile;
	if(SI_OK != xIniFile.LoadFile(szDestPath))
	{
		MessageBox("�޷����������ļ�", "����", MB_ICONERROR | MB_OK);
		return;
	}
	const char* pszUrl = xIniFile.GetValue("CONFIG", "GAMEROOMURL");
	if(NULL == pszUrl ||
		strlen(pszUrl) == 0)
	{
		MessageBox("�޷����������ļ�", "����", MB_ICONERROR | MB_OK);
		return;
	}
	m_xRegisterGameRoomUrl = pszUrl;

	RegisterGameRoomDlg dlg;
	int nDlgSel = dlg.DoModal();
	if(IDOK == nDlgSel)
	{
		m_xRegisterGameRoomName = dlg.m_xEditHostNameStr;
		m_xRegisterGameRoomPassword = dlg.m_xEditHostPasswordStr;
		m_xRegisterGameRoomIp = dlg.m_xEditHostIPStr;
		m_xRegisterGameRoomPort = dlg.m_xEditHostPortStr;

		//	��������תΪutf8
		m_xRegisterGameRoomName = GBKToUTF8(m_xRegisterGameRoomName);

		char szPort[10] = {0};
		m_xRegisterGameRoomPort = itoa(m_pxMainServer->GetListenPort(), szPort, 10);

		RegisterGameRoom();
		//	������ʱ��
		SetTimer(TIMER_REGISTERGS, 30 * 1000, NULL);
	}
}

void CServerDlg::RegisterGameRoom()
{
	char szUrl[MAX_PATH] = {0};
	sprintf(szUrl, "%s/registergs?address=%s&port=%s&note=%s&password=%s&online=%d&version=%s",
		m_xRegisterGameRoomUrl.c_str(),
		m_xRegisterGameRoomIp.c_str(),
		m_xRegisterGameRoomPort.c_str(),
		m_xRegisterGameRoomName.c_str(),
		m_xRegisterGameRoomPassword.c_str(),
		m_nOnlinePlayerCount,
		BACKMIR_CURVERSION);

	BMHttpManager::GetInstance()->DoGetRequestSync(szUrl, fastdelegate::bind(&CServerDlg::OnRegisterGsResult, this));
}

void CServerDlg::OnRegisterGsResult(const char *_pData, size_t _uLen)
{
	//	nothing
	if(NULL == _pData ||
		0 == _uLen)
	{
		if(0 != m_nGameRoomServerID)
		{
			AddTextToMessageBoardFmt("����Ϸ����������ʧȥ����...");
			m_nGameRoomServerID = 0;
		}
		else
		{
			AddTextToMessageBoardFmt("ע��������������ʧ��...");
		}
		GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
		return;
	}

	cJSON* pRoot = cJSON_Parse(_pData);
	if(NULL == pRoot)
	{
		return;
	}

	int nRet = cJSON_GetObjectItem(pRoot, "Result")->valueint;
	if(0 == nRet)
	{
		const char* pszServerID = cJSON_GetObjectItem(pRoot, "Msg")->valuestring;
		if(NULL != pszServerID)
		{
			if(0 == m_nGameRoomServerID)
			{
				AddTextToMessageBoardFmt("�ɹ�ע������Ϸ����");
			}
			m_nGameRoomServerID = atoi(pszServerID);
			GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE);
		}
	}
	else
	{
		const char* pszErrMsg = cJSON_GetObjectItem(pRoot, "Msg")->valuestring;
		if(NULL != pszErrMsg)
		{
			MessageBox(pszErrMsg, "����", MB_ICONERROR | MB_OK);
		}
		m_nGameRoomServerID = 0;
		GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
	}

	cJSON_Delete(pRoot);
	pRoot = NULL;
}

void CServerDlg::OnRemoveGsResult(const char *_pData, size_t _uLen)
{
	//	nothing
}

void CServerDlg::OnBnClickedButton7()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CConfigDlg dlg;
	if(IDOK == dlg.DoModal())
	{

	}
}
