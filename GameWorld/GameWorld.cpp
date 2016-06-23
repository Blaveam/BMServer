#include "../CMainServer/CMainServer.h"
#include "GameWorld.h"
#include "GameSceneManager.h"
#include "MonsterObject.h"
#include "ObjectValid.h"
#include "../Helper.h"
#include <process.h>
#include <Shlwapi.h>
#include <io.h>
#include <zlib.h>
#include <sstream>
#include "ExceptionHandler.h"
#include "OlShopManager.h"
#include "../../CommonModule/CRCVerifyThread/CRCCalc.h"
#include "../../CommonModule/SettingLoader.h"
#include "../../CommonModule/OfflineSellSystem.h"
#include "../../CommonModule/NotifySystem.h"
#include "../../CommonModule/HideAttribHelper.h"
#include "../../CommonModule/StoveManager.h"
//////////////////////////////////////////////////////////////////////////
//	For glog
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma comment(lib, "libglog_static_d.lib")
//#pragma comment(lib, "CommonModule_D.lib")
#else
#pragma comment(lib, "libglog.lib")
//#pragma comment(lib, "CommonModule.lib")
#endif
#pragma comment(lib, "shlwapi.lib")

//////////////////////////////////////////////////////////////////////////
extern HWND g_hServerDlg;
HeroObject* g_pxHeros[MAX_CONNECTIONS + 1] = {NULL};
std::vector<MagicInfo> g_xMagicInfoTable(MEFF_USERTOTAL);
std::map<int, int> g_xShowDonateTimeMap;
//////////////////////////////////////////////////////////////////////////
static const int s_nWeaponUpAttrib[] =
{
	ATTRIB_DC,
	ATTRIB_DC,
	ATTRIB_DC,
	ATTRIB_MC,
	ATTRIB_MC,
	ATTRIB_MC,
	ATTRIB_SC,
	ATTRIB_SC,
	ATTRIB_SC,
	//ATTRIB_LUCKY,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU
};

static const int s_nRingUpAttrib[] =
{
	ATTRIB_DC,
	ATTRIB_MC,
	ATTRIB_SC
};

static const int s_nBraUpAttrib[] =
{
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_DC,
	ATTRIB_MC,
	ATTRIB_SC
};

static const int s_nNeckUpAttrib[] =
{
	ATTRIB_DC,
	ATTRIB_DC,
	ATTRIB_DC,
	ATTRIB_SC,
	ATTRIB_SC,
	ATTRIB_SC,
	ATTRIB_MC,
	ATTRIB_MC,
	ATTRIB_MC,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_ACCU,
	ATTRIB_HIDE,
	ATTRIB_LUCKY
};

static const int s_nClothUpAttrib[] =
{
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_DC,
	ATTRIB_SC,
	ATTRIB_MC
};

static const int s_nHelmetUpAttrib[] =
{
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_DC,
	ATTRIB_MC,
	ATTRIB_SC
};

static const int s_nShoeUpAttrib[] =
{
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_DC,
	ATTRIB_MC,
	ATTRIB_SC
};

static const int s_nBeltUpAttrib[] =
{
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_DC,
	ATTRIB_MC,
	ATTRIB_SC
};

static const int s_nGemUpAttrib[] =
{
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_DC,
	ATTRIB_MC,
	ATTRIB_SC
};

static const int s_nMedalUpAttrib[] =
{
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_AC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_MAC,
	ATTRIB_DC,
	ATTRIB_MC,
	ATTRIB_SC
};
//////////////////////////////////////////////////////////////////////////
int GameWorld::m_nRetCode = 0;
static GameWorld* s_pGameWorld = NULL;
/************************************************************************/
/* 
/************************************************************************/
GameWorld::GameWorld()
	: m_xProcessD(MAX_COMMAND)
{
	//
	m_eState = WS_STOP;
	m_dwThreadID = 0;
	m_hThread = 0;
	m_dwWorkTotalTime = 0;
	m_bTerminate = false;
	m_bPause = false;
	m_xDatabase = DBThread::GetInstance();
	m_nOnlinePlayers = 0;
	//m_nRetCode = 0;
	m_nDropMultiple = m_nExprMultiple = 0;
	m_dwLastRecWatcherMsgTime = 0;
	m_nFinnalExprMulti = 0;
	EnableAutoReset();
	m_pRankListData = NULL;

	InitializeCriticalSection(&m_stCsProcess);

	m_bGenElitMons = false;
	m_bEnableOfflineSell = false;
	m_bEnableWorldNotify = false;

	m_dwLastBurstFireworkTime = 0;
	m_dwLastExpFireworkTime = 0;
	m_dwLastMagicDropFireworkTime = 0;
	m_nDifficultyLevel = 0;
	m_dwLastUpdateScriptEngineTime = GetTickCount();
}

GameWorld::~GameWorld()
{
	//
	m_bTerminate = true;
	/*while(1)
	{
		if(m_eState == WS_STOP)
		{
			break;
		}
	}*/
	DeleteCriticalSection(&m_stCsProcess);
	delete m_xDatabase;
	SAFE_DELETE_ARRAY(m_pRankListData);

	GameSceneManager::DestroyInstance();
}

GameWorld* GameWorld::GetInstancePtr()
{
	if(s_pGameWorld == NULL)
	{
		s_pGameWorld = new GameWorld;
	}
	return s_pGameWorld;
}

void GameWorld::DestroyInstance()
{
	if(NULL == s_pGameWorld)
	{
		return;
	}
	delete s_pGameWorld;
	s_pGameWorld = NULL;
}
//////////////////////////////////////////////////////////////////////////

/************************************************************************/
/* static unsigned int WorkThread(void* _pData)
/************************************************************************/
unsigned int THREAD_CALL GameWorld::WorkThread(void* _pData)
{
	RECORD_FUNCNAME_WORLD;
	//
	PROTECT_START_VM

	unsigned int uRetValue = 0;
	GameWorld* pIns = static_cast<GameWorld*>(_pData);
	pIns->m_eState = WS_WORKING;
	LOG(INFO) << "Main thread entered";

	//	now it is working
	DWORD dwLastWorkTime = ::GetTickCount();
	DWORD dwCurrentWorkTime = dwLastWorkTime;
	DWORD dwTimeInterval = 0;

	srand((unsigned int)time(NULL));

	//	Create the message loop
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	ZeroMemory(&msg, sizeof(MSG));

	PROTECT_END_VM

	unsigned int uRet = 0;

	//	��������������Ϣ
	//pIns->GetEventDispatcher()->DispatchEvent(kWorldEvent_StartRunning, NULL);
	pIns->DispatchLuaEvent(kLuaEvent_WorldStartRunning, NULL);

	while(TRUE)
	{
		//	Process thread message
		if(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			uRet = pIns->Thread_ProcessMessage(&msg);
		}
		if(0 != uRet)
		{
			LOG(ERROR) << "break with bad thread process";
			break;
		}

		//	working loop
		dwCurrentWorkTime = ::GetTickCount();
		dwTimeInterval = dwCurrentWorkTime - dwLastWorkTime;
		pIns->m_dwWorkTotalTime += dwTimeInterval;
		//	testing the terminate flag,if it is true, break the main loop thread
		if(pIns->m_bTerminate)
		{
			LOG(ERROR) << "break with terminate";
			break;
		}
		//	pause the thread if the pause flag is true
		if(pIns->m_bPause)
		{
			//	into sleep
			SleepEx(1, TRUE);
			continue;
		}

		//	updating the system first
		pIns->DoWork_System(dwTimeInterval);
		//	updating all objects
		pIns->DoWork_Objects(dwCurrentWorkTime);
		//	handle delayed process
		pIns->DoWork_DelayedProcess(dwTimeInterval);
		//	into sleep
		SleepEx(1, TRUE);

		//	Test
		if(dwCurrentWorkTime - dwLastWorkTime > 5000)
		{
			//	??
			//m_nRetCode = 3;
			//break;
			//ZeroMemory(GameWorld::GetInstancePtr(), sizeof(GameWorld));
			if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
			{
				//	nothing
			}
			else
			{
#ifdef _DEBUG
				LOG(INFO) << "quit with long loop time";
#endif
				DESTORY_GAMEWORLD;
			}
		}
		//	next loop
		dwLastWorkTime = dwCurrentWorkTime;
	}

	PROTECT_START_VM

	//	thread is done
	LOG(INFO) << "Main thread quit with return ["
		<< m_nRetCode << "]";
	pIns->m_eState = WS_STOP;

	PROTECT_END_VM

	_endthreadex(m_nRetCode);
	return m_nRetCode;
}

//////////////////////////////////////////////////////////////////////////

/************************************************************************/
/* unsigned int Run()
/************************************************************************/
unsigned int GameWorld::Run()
{
	if(m_eState == WS_WORKING)
	{
		LOG(ERROR) << "Main loop thread is already running!";
		return 0;
	}
	//	Set the receive thread msg time
	m_dwLastRecWatcherMsgTime = GetTickCount();

	//	Set the global table to zero
	memset(g_pxHeros, 0, sizeof(g_pxHeros));
	//LoadMagicInfo();

	//	First open database
	char szPath[MAX_PATH];

	ItemAttrib item;
	item.id = 3333384305;
	char szName[20] = {-104, 110, 67, 18, 6, 0, 4, 0, -7, 1, 12, 8, 96, 66, -22, 17, 72, 3, 62};
	strcpy(item.name, szName);
	char szDecrypt[20];
	ObjectValid::GetItemName(&item, szDecrypt);

	unsigned int nDecryptId = ObjectValid::GetItem_ID(&item);

	unsigned int nId = DecryptValue(ATB_ID_MASK, 3333384305);
	nId = DecryptValue(ATB_ID_MASK, 3333384267);
	/*GetModuleFileName(NULL, szPath, MAX_PATH);
	PathRemoveFileSpec(szPath);
	strcat(szPath, "\\Help\\legend.bm");
	if(!m_xDatabase->ConnectDB(szPath))
	{
		LOG(ERROR) << "Can not connect to db[" << szPath << "]";
		return 0;
	}
	if(m_xDatabase->LoadItemsPrice())
	{
		AddInfomation("��ȡװ���۸�ɹ�");
	}
	else
	{
		AddInfomation("��ȡװ���۸�ʧ��");
	}
	if(m_xDatabase->LoadMagicAttrib())
	{
		AddInfomation("��ȡħ����Ϣ�ɹ�");
	}
	else
	{
		AddInfomation("��ȡħ����Ϣʧ��");
	}*/
 	if(!m_xDatabase->Run())
 	{
 		//	Error occur
 		LOG(ERROR) << "Can not start database!";
 		return 0;
 	}
	//	Load script
	GetModuleFileName(NULL, szPath, MAX_PATH);
	PathRemoveFileSpec(szPath);

	//	no reset
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
	{
		DisableAutoReset();
	}

	//	load offline sell system
	if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN &&
		GameWorld::GetInstancePtr()->IsEnableOfflineSell())
	{
		char szOfflineSellFile[MAX_PATH];
		sprintf(szOfflineSellFile, "%s/login/offlinesell.db",
			GetRootPath());
		if(!OfflineSellSystem::GetInstance()->Initialize(szOfflineSellFile))
		{
			LOG(ERROR) << "Can not initialize offline sell system";
			return 0;
		}
		OfflineSellSystem::GetInstance()->CopyFromSQL();
	}

	if(GameWorld::GetInstance().IsEnableWorldNotify())
	{
		char szOfflineSellFile[MAX_PATH];
		sprintf(szOfflineSellFile, "%s/settings/notify.txt",
			GetRootPath());
		if(!NotifySystem::GetInstance()->Initialize(szOfflineSellFile))
		{
			LOG(ERROR) << "Can not initialize world notify";
			return 0;
		}
	}

	//	stove manager
	if(!StoveManager::GetInstance()->Init())
	{
		LOG(ERROR) << "Can't initialize stove manager";
		return 0;
	}

	//	load settings
	//m_bGenElitMons = (SettingLoader::GetInstance()->GetIntValue("GENELITEMONS") != 0);
#ifdef _DEBUG
	strcat(szPath, "\\Help\\");
	m_xScript.SetModulePath(szPath);
	m_xScript.SetLuaLoadPath(szPath);
#else
	strcat(szPath, "\\Help\\dog.idx");
	m_xScript.SetModulePath(szPath, LOADMODE_ZIP);
	m_xScript.SetLuaLoadPath(szPath);
#endif

	if(!LoadScript())
	{
#ifdef _DEBUG
		LOG(ERROR) << "Can not load the world script file";
#endif
		return 0;
	}

	LoadBlackList();

	m_dwWorkTotalTime = 0;
	unsigned int uThreadID = 0;

	m_hThread = (HANDLE)_beginthreadex(NULL,
		0,
		&GameWorld::WorkThread,
		this,
		0,
		&uThreadID);
	m_dwThreadID = (DWORD)uThreadID;

	if(m_hThread == 0)
	{
		//	create thread unsuccessfully
		LOG(FATAL) << "Create main loop thread unsuccessfully! Error code:["
			<< GetLastError() << "]";
		return 0;
	}
	else
	{
		//	create thread successfully
		LOG(INFO) << "Create main loop thread successfully! Now it is running!";
#ifdef _BUILD_VERSION
		LOG(INFO) << "Build version:" << _BUILD_VERSION;
#endif
		return 1;
	}
}

/************************************************************************/
/* void Initialize(const char* _pszDestFile = NULL)
/*
/*	if _pszDestFile is NULL,the log file will be set as ./WorldLog,
/*	otherwise, the log file will be set as the path that the paramters input
/************************************************************************/
void GameWorld::Initialize(const char* _pszDestFile /* = NULL */)
{
	char szModule[MAX_PATH];
	::GetModuleFileName(NULL,
		szModule,
		MAX_PATH);

	google::InitGoogleLogging(szModule);

	if(NULL == _pszDestFile)
	{
		PathRemoveFileSpec(szModule);
		strcat(szModule, "\\WorldLog");
		_pszDestFile = szModule;
	}

	//	if the path not exists, create it before calling SetLogDestination
	if(!PathFileExists(_pszDestFile))
	{
		BOOL bSuc = CreateDirectory(_pszDestFile,
			NULL);
		if(!bSuc)
		{
			::OutputDebugString("Error when creating directory");
		}
	}
	strcat(szModule, "\\");
	google::SetLogDestination(google::GLOG_INFO, _pszDestFile);
}


//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* void DoWork_Objects(DWORD _dwTick)
/************************************************************************/
void GameWorld::DoWork_Objects(DWORD _dwTick)
{
	GameSceneManager::GetInstance()->Update(_dwTick);
	m_xCronScheduler.Update();
}

/************************************************************************/
/* void DoWork_System(DWORD _dwTick)
/************************************************************************/
void GameWorld::DoWork_System(DWORD _dwTick)
{
	if(IsEnableWorldNotify())
	{
		CCharPtrVector xCCharPtrVector;
		if(NotifySystem::GetInstance()->QueryNotifyItem(xCCharPtrVector) > 0)
		{
			for(int i = 0; i < xCCharPtrVector.size(); ++i)
			{
				//GameSceneManager::GetInstance()->SendRawSystemMessageAllScene(xCCharPtrVector[i]);
				PkgSystemNotifyNot not;
				not.dwTimes = 1;
				not.xMsg = xCCharPtrVector[i];
				not.dwColor = 0xffffff00;
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << not;
				GameSceneManager::GetInstance()->BroadcastPacketAllScene(&g_xThreadBuffer);
			}
		}
	}

	//	�̻�
	if(GetTickCount() > m_dwLastBurstFireworkTime &&
		m_dwLastBurstFireworkTime != 0)
	{
		m_dwLastBurstFireworkTime = 0;
		GameSceneManager::GetInstance()->SendSystemNotifyAllScene("�񻨹��ڣ�����ָ�����");
	}
	if(GetTickCount() > m_dwLastExpFireworkTime &&
		m_dwLastExpFireworkTime != 0)
	{
		m_dwLastExpFireworkTime = 0;
		GameSceneManager::GetInstance()->SendSystemNotifyAllScene("�񻨹��ڣ����ʻָ�����");
	}
	if(GetTickCount() > m_dwLastMagicDropFireworkTime &&
		m_dwLastMagicDropFireworkTime != 0)
	{
		m_dwLastMagicDropFireworkTime = 0;
		GameSceneManager::GetInstance()->SendSystemNotifyAllScene("�񻨹��ڣ�ħ��װ���ӳɻָ�����");
	}

	//////////////////////////////////////////////////////////////////////////
	//	���½ű�����
	if(GetTickCount() - m_dwLastUpdateScriptEngineTime > 1000)
	{
		m_dwLastUpdateScriptEngineTime = GetTickCount();
		m_xScript.DispatchEvent(kLuaEvent_WorldUpdate, NULL);
	}
}

/************************************************************************/
/* void OnMessage(unsigned int _uId, const void* _pData, unsigned int _uLen)
/*	_uId : not connection index
/************************************************************************/
void GameWorld::OnMessage(unsigned int _uId, const void* _pData, unsigned int _uLen)
{
	//	Not for use!
	//GameObject* pObj = GetPlayer(_uId);
// 	GameObject* pObj = GameSceneManager::GetInstance()->GetPlayer(_uId);
// 	if(NULL == pObj)
// 	{
// 		LOG(WARNING) << "The target hero doesn't exist!";
// 		return;
// 	}
// 
// 	if(pObj->GetType() == SOT_HERO)
// 	{
// 		//	Only hero can receive message
// 		HeroObject* pHero = static_cast<HeroObject*>(pObj);
// 		if(0 == pHero->PushMessage(_pData, _uLen))
// 		{
// 			LOG(FATAL) << "Buffer error when writing to the buffer of user["
// 				<< _uId << "]";
// 		}
// 	}
// 	else
// 	{
// 		//LOG(ERROR) << "Some message have send to the object which is not a hero";
// 	}
}

void GameWorld::OnMessage(unsigned int _dwIndex, unsigned int _uId, ByteBuffer* _xBuffer)
{
	//GameObject* pObj = GetPlayer(_uId);
	//	below maybe not thread-safe
	//	another thread may have insert or remove operation
	/*
	GameObject* pObj = GameSceneManager::GetInstance()->GetPlayer(_uId);
		if(NULL == pObj)
		{
			LOG(WARNING) << "The target hero doesn't exist!";
			return;
		}*/
	GameObject* pObj = NULL;
	if(_dwIndex <= MAX_CONNECTIONS)
	{
		pObj = g_pxHeros[_dwIndex];
	}
	if(pObj == NULL)
	{
		pObj = GameSceneManager::GetInstance()->GetPlayer(_uId);
	}

	if(NULL == pObj)
	{
		return;
	}

	if(pObj->GetType() == SOT_HERO)
	{
		//	Only hero can receive message
		HeroObject* pHero = static_cast<HeroObject*>(pObj);
		if(0 == pHero->PushMessage(_xBuffer))
		{
			LOG(FATAL) << "Buffer error when writing to the buffer of user["
				<< _uId << "]";
		}
	}
	else
	{
		//LOG(ERROR) << "Some message have send to the object which is not a hero";
	}
}
/************************************************************************/
/* unsigned int Broadcast(ByteBuffer* _pBuf)
/************************************************************************/
unsigned int GameWorld::Broadcast(ByteBuffer* _pBuf)
{
	DWORD dwCounter = 0;

// 	for(ObjectMap::const_iterator iter = m_xPlayers.begin();
// 		iter != m_xPlayers.end();
// 		++iter)
// 	{
// 		if(0 == SendBuffer(iter->second->GetUserIndex(),
// 			_pBuf))
// 		{
// 			LOG(ERROR) << "Broadcast packet error!";
// 		}
// 		else
// 		{
// 			++dwCounter;
// 		}
// 	}

	return dwCounter;
}

/************************************************************************/
/* unsigned int BroadcastExcept(GameObject* _pObj, ByteBuffer* _pBuf)
/************************************************************************/
unsigned int GameWorld::BroadcastExcept(GameObject* _pObj, ByteBuffer* _pBuf)
{
	DWORD dwCounter = 0;

// 	if(_pObj == NULL)
// 	{
// 		return Broadcast(_pBuf);
// 	}
// 	else
// 	{
// 		if(_pObj->GetType() != SOT_HERO)
// 		{
// 			return 0;
// 		}
// 
// 		for(ObjectMap::const_iterator iter = m_xPlayers.begin();
// 			iter != m_xPlayers.end();
// 			++iter)
// 		{
// 			if(iter->second == _pObj)
// 			{
// 				continue;
// 			}
// 			if(0 == SendBuffer(iter->second->GetUserIndex(),
// 				_pBuf))
// 			{
// 				LOG(ERROR) << "Broadcast packet error!";
// 			}
// 			else
// 			{
// 				++dwCounter;
// 			}
// 		}
// 	}

	return dwCounter;
}
/************************************************************************/
/* unsigned int BroadcastRange(ByteBuffer* _pBuf, DWORD _dwSrcX, DWORD _dwSrcY, DWORD _dwOftX = 10, DWORD _dwOftY = 10)
/************************************************************************/
unsigned int GameWorld::BroadcastRange(ByteBuffer* _pBuf, DWORD _dwSrcX, DWORD _dwSrcY, DWORD _dwOftX /* = 10 */, DWORD _dwOftY /* = 10 */)
{
	DWORD dwCounter = 0;
// 	RECT rcRange;
// 	rcRange.left = _dwSrcX - _dwOftX;
// 	rcRange.right = _dwOftX + _dwSrcX;
// 	if(rcRange.left == 0)
// 	{
// 		rcRange.left = 0;
// 	}
// 	if(rcRange.right == 0)
// 	{
// 		rcRange.right = 0;
// 	}
// 	rcRange.top = _dwSrcY - _dwOftY;
// 	rcRange.bottom = _dwSrcY + _dwOftY;
// 	if(rcRange.top == 0)
// 	{
// 		rcRange.top = 0;
// 	}
// 	if(rcRange.bottom == 0)
// 	{
// 		rcRange.bottom = 0;
// 	}

// 	for(ObjectMap::const_iterator iter = m_xPlayers.begin();
// 		iter != m_xPlayers.end();
// 		++iter)
// 	{
// 		if(PtInRect(&rcRange, iter->second->m_ptPosition))
// 		{
// 			if(0 == SendBuffer(iter->second->GetUserIndex(),
// 				_pBuf))
// 			{
// 				LOG(ERROR) << "Broadcast packet error!";
// 			}
// 			else
// 			{
// 				++dwCounter;
// 			}
// 		}
// 	}

	return dwCounter;
}
/************************************************************************/
/* unsigned int BroadcastRangeExcept(GameObject* _pObj, ByteBuffer* _pBuf, DWORD _dwSrcX, DWORD _dwSrcY, DWORD _dwOftX = 10, DWORD _dwOftY = 10)
/************************************************************************/
unsigned int GameWorld::BroadcastRangeExcept(GameObject* _pObj, ByteBuffer* _pBuf, DWORD _dwSrcX, DWORD _dwSrcY, DWORD _dwOftX /* = 10 */, DWORD _dwOftY /* = 10 */)
{
	DWORD dwCounter = 0;

// 	if(_pObj == NULL)
// 	{
// 		return BroadcastRange(_pBuf, _dwSrcX, _dwSrcY, _dwOftX, _dwOftY);
// 	}
// 	else
// 	{
// 		RECT rcRange;
// 		rcRange.left = _dwSrcX - _dwOftX;
// 		rcRange.right = _dwOftX + _dwSrcX;
// 		if(rcRange.left == 0)
// 		{
// 			rcRange.left = 0;
// 		}
// 		if(rcRange.right == 0)
// 		{
// 			rcRange.right = 0;
// 		}
// 		rcRange.top = _dwSrcY - _dwOftY;
// 		rcRange.bottom = _dwSrcY + _dwOftY;
// 		if(rcRange.top == 0)
// 		{
// 			rcRange.top = 0;
// 		}
// 		if(rcRange.bottom == 0)
// 		{
// 			rcRange.bottom = 0;
// 		}

// 		for(ObjectMap::const_iterator iter = m_xPlayers.begin();
// 			iter != m_xPlayers.end();
// 			++iter)
// 		{
// 			if(iter->second == _pObj)
// 			{
// 				continue;
// 			}
// 			if(PtInRect(&rcRange, iter->second->m_ptPosition))
// 			{
// 				if(0 == SendBuffer(iter->second->GetUserIndex(),
// 					_pBuf))
// 				{
// 					LOG(ERROR) << "Broadcast packet error!";
// 				}
// 				else
// 				{
// 					++dwCounter;
// 				}
// 			}
// 		}
//	}

	return dwCounter;
}

/************************************************************************/
/* void DoWork_DelayedProcess(DWORD _dwTick)
/************************************************************************/
void GameWorld::DoWork_DelayedProcess(DWORD _dwTick)
{
	RECORD_FUNCNAME_WORLD;
	RECORD_FUNCLINE_WORLD;

	DelayedProcess stDp;
	LockProcess();

	RECORD_FUNCLINE_WORLD;

	while(m_xProcessD.GetLength())
	{
		try
		{
			m_xProcessD >> stDp;

			if(stDp.uOp > DP_GAME_START &&
				stDp.uOp < DP_GAME_END)
			{
				DoDelayGame(stDp);
				RECORD_FUNCLINE_WORLD;
			}
			else if(stDp.uOp > DP_DATABASE_START &&
				stDp.uOp < DP_DATABASE_END)
			{
				DoDelayDatabase(stDp);
				RECORD_FUNCLINE_WORLD;
			}
			else if(stDp.uOp > DP_SYSTEM_START &&
				stDp.uOp < DP_SYSTEM_END)
			{
				DoDelaySystem(stDp);
				RECORD_FUNCLINE_WORLD;
			}
		}
		catch(std::exception& e)
		{
			ALERT_MSGBOX("Delayed buffer processing error!");
		}
		catch(...)
		{
			LOG(ERROR) << "Fatal error";
		}
	}
	RECORD_FUNCLINE_WORLD;
	m_xProcessD.Reset();

	if(GetTickCount() - m_dwLastRecWatcherMsgTime > 30000)
	{
#ifndef _DISABLE_CONSOLE
		g_xConsole.CPrint("Watcher thread abnormal!!!");
		m_dwLastRecWatcherMsgTime = GetTickCount();
#endif
		if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
		{
			LOG(ERROR) << "DESTORY_GAMESCENE";
		}
		else
		{
			DESTORY_GAMESCENE;
		}
	}

	RECORD_FUNCLINE_WORLD;
	UnlockProcess();
	RECORD_FUNCLINE_WORLD;
}

void GameWorld::DoDelayGame(const DelayedProcess &_dp)
{
	RECORD_FUNCNAME_WORLD;
	RECORD_FUNCLINE_WORLD;

	switch(_dp.uOp)
	{
	case DP_USERLOGIN:
		{
			RECORD_FUNCLINE_WORLD;

			GameObject* pObj = NULL;
			memcpy(&pObj, &_dp.uParam0, sizeof(void*));
			HeroObject* pHero = static_cast<HeroObject*>(pObj);

			if(pObj != NULL)
			{
				pObj->SetID(GenerateObjectID());

				if(NULL == pObj->GetLocateScene())
				{
					LOG(ERROR) << "Nonexistent scene!";
					//SAFE_DELETE(pObj);
					PostMessage(g_hServerDlg, WM_REMOVEPLAYER, (WPARAM)pHero->GetUserIndex(), (LPARAM)pObj);
					return;
				}

				if(pObj->GetObject_HP() == 0)
				{
					//	Dead
					GameScene* pHomeScene = pHero->GetHomeScene();
					if(pHomeScene != NULL)
					{
						pHero->SetMapID(pHomeScene->GetMapID());
						pHero->GetUserData()->wCoordX = pHomeScene->GetCityCenterX();
						pHero->GetUserData()->wCoordY = pHomeScene->GetCityCenterY();
						pHero->SetObject_HP(10);
					}
				}
				else
				{
					//	A instance scene?
					//if(pObj->GetLocateScene()->IsInstance())
					if(pObj->GetLocateScene()->IsTreasureMap())
					{
						GameScene* pHomeScene = pHero->GetHomeScene();
						if(pHomeScene != NULL)
						{
							pHero->SetMapID(pHomeScene->GetMapID());
							pHero->GetUserData()->wCoordX = pHomeScene->GetCityCenterX();
							pHero->GetUserData()->wCoordY = pHomeScene->GetCityCenterY();
						}
					}
					//	A cannot save position scene?
					if(!pObj->GetLocateScene()->CanSaveAndStay())
					{
						GameScene* pHomeScene = pHero->GetHomeScene();
						if(pHomeScene != NULL)
						{
							pHero->SetMapID(pHomeScene->GetMapID());
							pHero->GetUserData()->wCoordX = pHomeScene->GetCityCenterX();
							pHero->GetUserData()->wCoordY = pHomeScene->GetCityCenterY();
						}
					}
					//	blocked by something?
					if(!pObj->GetLocateScene()->CanThrough(pObj->GetUserData()->wCoordX, pObj->GetUserData()->wCoordY))
					{
						GameScene* pHomeScene = pHero->GetHomeScene();
						if(pHomeScene != NULL)
						{
							pHero->SetMapID(pHomeScene->GetMapID());
							pHero->GetUserData()->wCoordX = pHomeScene->GetCityCenterX();
							pHero->GetUserData()->wCoordY = pHomeScene->GetCityCenterY();
						}
					}
					//	Around blocked?
					bool bAroundBlocked = true;
					int nAroundX = 0;
					int nAroundY = 0;

					for(int i = 0; i < 8; ++i)
					{
						nAroundX = pHero->GetUserData()->wCoordX + g_nMoveOft[i * 2];
						nAroundY = pHero->GetUserData()->wCoordY + g_nMoveOft[i * 2 + 1];

						if(pHero->GetLocateScene()->CanThrough(nAroundX, nAroundY))
						{
							bAroundBlocked = false;
							break;
						}
					}

					if(bAroundBlocked)
					{
						GameScene* pHomeScene = pHero->GetHomeScene();
						if(pHomeScene != NULL)
						{
							pHero->SetMapID(pHomeScene->GetMapID());
							pHero->GetUserData()->wCoordX = pHomeScene->GetCityCenterX();
							pHero->GetUserData()->wCoordY = pHomeScene->GetCityCenterY();
						}
					}
				}

				bool bLoginSuccess = false;

				//	����Ƿ��ܽ�����Ϸ����
				HeroObject* pOldHero = (HeroObject*)GameSceneManager::GetInstance()->GetPlayerByName(pObj->GetUserData()->stAttrib.name);
				if(NULL != pOldHero)
				{
					//	�޷�����
					pOldHero->SendSystemMessage("�����˺ű����˵�¼");
					PostMessage(g_hServerDlg, WM_CLOSECONNECTION, pOldHero->GetUserIndex(), 0);
				}
				else
				{
					if(GameSceneManager::GetInstance()->InsertPlayer(pObj))
					{
						bLoginSuccess = true;

						if(false == pHero->AcceptLogin(_dp.uParam1 == 0))
						{
							GameSceneManager::GetInstance()->RemovePlayer(pObj->GetID());
						}
						else
						{
							char szName[20];
							if(pObj->IsEncrypt())
							{
								ObjectValid::GetItemName(&pObj->GetUserData()->stAttrib, szName);
							}
							else
							{
								strcpy(szName, pObj->GetUserData()->stAttrib.name);
							}

							UserInfo info;
							info.xName = szName;
							info.uHandlerID = pObj->GetID();
							g_xUserInfoList.push_back(info);
						}
					}
				}

				if(!bLoginSuccess)
				{
					//SAFE_DELETE(pObj);
					PostMessage(g_hServerDlg, WM_REMOVEPLAYER, (WPARAM)pHero->GetUserIndex(), (LPARAM)pObj);
				}
			}
		}break;
	case DP_USERLOGOUT:
		{
			RECORD_FUNCLINE_WORLD;
			if(GameSceneManager::GetInstance()->RemovePlayer(_dp.uParam0))
			{
				LOG(INFO) << "Delete object" << "[" << _dp.uParam0 << "]";
			}
			else
			{
				LOG(INFO) << "Can't delete object" << "[" << _dp.uParam0 << "]";
			}
			RECORD_FUNCLINE_WORLD;
			//GameSceneManager::GetInstance()->AllMonsterHPToFull();
		}break;
	default:
		{
			LOG(WARNING) << "Unknown world process code[" << _dp.uOp << "]";
		}break;
	}
}

void GameWorld::DoDelayDatabase(const DelayedProcess &_dp)
{
	RECORD_FUNCNAME_WORLD;

	switch(_dp.uOp)
	{
	case DP_ITEMLOADED:
		{
			DBOperationParam* pParam = (DBOperationParam*)_dp.uParam0;
			/*
			if(LOWORD(pParam->dwParam[1]) == IE_ADDPLAYERITEM)
						{
							//	add player's item
							PkgPlayerGainItemNtf ntf;
							ItemAttrib* pItem = (ItemAttrib*)pParam->dwParam[0];
							pItem->tag = GenerateItemTag();
							memcpy(&ntf.stItem, pItem, sizeof(ItemAttrib));
							GameObject* pObj = (GameObject*)pParam->dwParam[2];
							ntf.uTargetId = pObj->GetID();
							g_xThreadBuffer.Reset();
							g_xThreadBuffer << ntf;
							//SendBuffer(pObj->GetUserIndex(), &g_xThreadBuffer);
						}*/
			
			if(LOWORD(pParam->dwParam[2]) == IE_ADDGROUNDITEM)
			{
				//	add ground item
				GroundItem* pItem = (GroundItem*)pParam->dwParam[0];
				pItem->stAttrib.tag = GenerateItemTag();
				GameScene* pScene = GameSceneManager::GetInstance()->GetScene(HIWORD(pParam->dwParam[2]));
				if(pScene)
				{
					UpgradeItems(&pItem->stAttrib);
					ObjectValid::EncryptAttrib(&pItem->stAttrib);
					if(pScene->InsertItem(pItem, false))
					{
#ifdef _DEBUG
						LOG(INFO) << "��ͼ" << HIWORD(pParam->dwParam[2]) << "����Item[" << GETITEMATB(&pItem->stAttrib, ID) << "]�ɹ�";
#endif
						//UpgradeItems(&pItem->stAttrib);
						//	Broadcast it
					}
					else
					{
						delete pItem;
					}
				}
			}
			else if(LOWORD(pParam->dwParam[2]) == IE_ADDPLAYERITEM)
			{
				//	Add player item
				ItemAttrib* pItem = (ItemAttrib*)pParam->dwParam[0];
				pItem->tag = GenerateItemTag();
				SET_FLAG(pItem->atkPois, POIS_MASK_BIND);
				ObjectValid::EncryptAttrib(pItem);
				PkgPlayerGainItemNtf ntf;
				ntf.uTargetId = HIWORD(pParam->dwParam[2]);
				ntf.stItem = *pItem;
				ObjectValid::DecryptAttrib(&ntf.stItem);
				g_xThreadBuffer.Reset();
				g_xThreadBuffer << ntf;
				GameScene* pScene = GameSceneManager::GetInstance()->GetScene(LOWORD(pParam->dwParam[1]));
				if(pScene != NULL)
				{
					HeroObject* pHero = (HeroObject*)pScene->GetPlayerWithoutLock(HIWORD(pParam->dwParam[2]));
					if(pHero)
					{
						//pHero->SendPlayerBuffer(g_xThreadBuffer);
						SendBuffer(pHero->GetUserIndex(), &g_xThreadBuffer);
					}
				}
			}
			else if(LOWORD(pParam->dwParam[2]) == IE_ADDBALEITEM)
			{
				//	Add player item
				ItemAttrib* pItem = (ItemAttrib*)pParam->dwParam[0];
				pItem->tag = GenerateItemTag();
				PkgPlayerGetBaleItemAck ack;
				ack.dwTagBeg = pItem->tag;
				ack.uTargetId = HIWORD(pParam->dwParam[2]);
				ack.dwID = pItem->id;
				GameScene* pScene = GameSceneManager::GetInstance()->GetScene(LOWORD(pParam->dwParam[1]));
				if(pScene != NULL)
				{
					HeroObject* pHero = (HeroObject*)pScene->GetPlayerWithoutLock(HIWORD(pParam->dwParam[2]));
					if(pHero)
					{
						for(int i = 0; i < 5; ++i)
						{
							pItem->tag = GenerateItemTag();
							pHero->AddBagItem(pItem);
						}
						pHero->SendPacket(ack);
						//SendBuffer(pHero->GetUserIndex(), &g_xThreadBuffer);
					}
				}
			}
			else if(LOWORD(pParam->dwParam[2]) == IE_BUYSHOPITEM)
			{
				//	check money
				ItemAttrib* pItem = (ItemAttrib*)pParam->dwParam[0];
				SET_FLAG(pItem->atkPois, POIS_MASK_BIND);
				ObjectValid::EncryptAttrib(pItem);
				GameScene* pScene = GameSceneManager::GetInstance()->GetScene(LOWORD(pParam->dwParam[1]));
				if(pScene != NULL)
				{
					HeroObject* pHero = (HeroObject*)pScene->GetPlayerWithoutLock(HIWORD(pParam->dwParam[2]));
					if(pHero)
					{
#ifdef _DEBUG
						//if(pHero->GetMoney() >= pItem->price)
						{
							//int nMoney = pHero->GetMoney() - pItem->price;
							//pHero->SetMoney(nMoney);
							pItem->tag = GenerateItemTag();

							PkgPlayerGainItemNtf ntf;
							ntf.uTargetId = HIWORD(pParam->dwParam[2]);
							ntf.stItem = *pItem;
							ObjectValid::DecryptAttrib(&ntf.stItem);
							g_xThreadBuffer.Reset();
							g_xThreadBuffer << ntf;
							pHero->SendPlayerBuffer(g_xThreadBuffer);

							PkgPlayerUpdateAttribNtf udntf;
							udntf.uTargetId = HIWORD(pParam->dwParam[2]);
							//udntf.dwParam = MAKELONG(UPDATE_MONEY, 0);
							udntf.bType = UPDATE_MONEY;
							udntf.dwParam = pHero->GetMoney();
							g_xThreadBuffer.Reset();
							g_xThreadBuffer << udntf;
							pHero->SendPlayerBuffer(g_xThreadBuffer);
#ifdef _DEBUG
							//LOG(INFO) << "���:" << pHero->GetUserData()->stAttrib.name << "������[" << pItem->name << "]";
#endif
						}
						//else
						{
							//ZeroMemory(pItem, sizeof(ItemAttrib));
						}
#else
						if(pHero->GetMoney() >= pItem->price)
						{
							int nMoney = pHero->GetMoney() - pItem->price;
							pHero->SetMoney(nMoney);
							pItem->tag = GenerateItemTag();

							PkgPlayerGainItemNtf ntf;
							ntf.uTargetId = HIWORD(pParam->dwParam[2]);
							ntf.stItem = *pItem;
							ObjectValid::DecryptAttrib(&ntf.stItem);
							g_xThreadBuffer.Reset();
							g_xThreadBuffer << ntf;
							pHero->SendPlayerBuffer(g_xThreadBuffer);

							PkgPlayerUpdateAttribNtf udntf;
							udntf.uTargetId = HIWORD(pParam->dwParam[2]);
							//udntf.dwParam = MAKELONG(UPDATE_MONEY, 0);
							udntf.bType = UPDATE_MONEY;
							udntf.dwParam = pHero->GetMoney();
							g_xThreadBuffer.Reset();
							g_xThreadBuffer << udntf;
							pHero->SendPlayerBuffer(g_xThreadBuffer);

							//LOG(INFO) << "���:" << pHero->GetUserData()->stAttrib.name << "������[" << pItem->name << "]";
						}
						else
						{
							ZeroMemory(pItem, sizeof(ItemAttrib));
							ObjectValid::EncryptAttrib(pItem);
						}
#endif
					}
				}
			}
			delete pParam;
		}break;
	case DP_MONSLOADED:
		{
			DBOperationParam* pParam = (DBOperationParam*)_dp.uParam0;
			GameObject* pObj = (GameObject*)pParam->dwParam[0];
			GameScene* pScene = GameSceneManager::GetInstance()->GetScene(pObj->GetMapID());
			if(pScene &&
				pObj)
			{
				//	Encrypt it
				pObj->EncryptObject();
				pScene->InsertNPC(pObj);
				if(pObj->GetType() == SOT_NPC)
				{
#ifdef _DEBUG
					LOG(INFO) << "��ͼ[" << pObj->GetMapID() << "]����NPC[" << pObj->GetObject_ID() << "]�ɹ�";
#endif
				}
				else if(pObj->GetType() == SOT_MONSTER)
				{
#ifdef _DEBUG
					LOG(INFO) << "��ͼ[" << pObj->GetMapID() << "]����Monster[" << pObj->GetObject_ID() << "]�ɹ�";
#endif
					MonsterObject* pMonster = static_cast<MonsterObject*>(pObj);
					pMonster->UpdateCostTime();
					pMonster->GetValidCheck()->UpdateAllAttrib();

					if(pMonster->GetMaster() != NULL)
					{
						PkgPlayerUpdateAttribNtf ntf;
						ntf.uTargetId = pMonster->GetID();
						ntf.bType = UPDATE_MASTER;
						ntf.dwParam = pMonster->GetMaster()->GetID();
						g_xThreadBuffer.Reset();
						g_xThreadBuffer << ntf;
						pScene->BroadcastPacket(&g_xThreadBuffer);

						if(pMonster->GetObject_ID() == MONSTER_SHENSHOU ||
							pMonster->GetObject_ID() == 62)
						{
							int nSC = 0;

							if(pMonster->GetMaster()->GetType() == SOT_HERO)
							{
								HeroObject* pHero = static_cast<HeroObject*>(pMonster->GetMaster());
								{
									int nLevel = 0;
									if(pMonster->GetObject_ID() == MONSTER_SHENSHOU)
									{
										nLevel = pHero->GetUserMagic(MEFF_SUMMON)->bLevel;
									}
									else if(pMonster->GetObject_ID() == 62)
									{
										nLevel = pHero->GetUserMagic(MEFF_SUPERSUMMON)->bLevel;
									}
									nSC = pHero->GetRandomAbility(AT_SC);
									if(nLevel > 0)
									{
										pMonster->SetInitLevel(nLevel);
										pMonster->SetUpgradeLevel(nLevel);

										for(int i = 0; i < nLevel; ++i)
										{
											pMonster->SlaveLevelUp();
										}

										ntf.bType = UPDATE_SLAVELV;
										ntf.dwParam = pMonster->GetObject_MaxAC();
										ntf.uTargetId = pMonster->GetID();
										g_xThreadBuffer.Reset();
										g_xThreadBuffer << ntf;
										pMonster->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
									}
								}
							}
						}
						else if(pMonster->GetObject_ID() == MONSTER_KULOU)
						{
							HeroObject* pHero = static_cast<HeroObject*>(pMonster->GetMaster());
							{
								int nLevel = pHero->GetUserMagic(MEFF_KULOU)->bLevel;
								if(nLevel > 0)
								{
									pMonster->SetInitLevel(nLevel);
									pMonster->SetUpgradeLevel(nLevel);

									ntf.bType = UPDATE_SLAVELV;
									ntf.dwParam = pMonster->GetObject_MaxAC();
									ntf.uTargetId = pMonster->GetID();
									g_xThreadBuffer.Reset();
									g_xThreadBuffer << ntf;
									pMonster->GetLocateScene()->BroadcastPacket(&g_xThreadBuffer);
								}
							}
						}
					}
				}
			}
			delete pParam;
		}break;
	case DP_DROPITEMEXLOADED:
		{
			DBOperationParam* pParam = (DBOperationParam*)_dp.uParam0;
			std::list<int>* pItems = (std::list<int>*)pParam->dwParam[2];

			if(!pItems->empty())
			{
				GameScene* pScene = GameSceneManager::GetInstance()->GetScene(HIWORD(pParam->dwParam[3]));

				if(pScene)
				{
					std::list<int>::const_iterator begIter = pItems->begin();
					std::list<int>::const_iterator endIter = pItems->end();
					GroundItem* pGroundItem = NULL;

					for(begIter;
						begIter != endIter;
						++begIter)
					{
						pGroundItem = (GroundItem*)*begIter;
						bool bItemValid = true;

						if(HeroObject::IsEquipItem(pGroundItem->stAttrib.type))
						{
							int nQualityIndex = GetItemUpgrade(pGroundItem->stAttrib.level);
							if(nQualityIndex > 5 &&
								nQualityIndex <= 8)
							{
								//	֪ͨ����
								PkgSystemNotifyNot not;

								switch(nQualityIndex)
								{
								case 6:
									{
										not.xMsg = "�Ƿ���Ʒ[";
									}break;
								case 7:
									{
										not.xMsg = "��˵��Ʒ[";
									}break;
								case 8:
									{
										not.xMsg = "ʷʫ��Ʒ[";
									}break;
								}

								//not.xMsg = "ʷʫ��Ʒ[";
								not.xMsg += pGroundItem->stAttrib.name;
								not.xMsg += "]������[";
								not.xMsg += GameSceneManager::GetInstance()->GetMapChName(HIWORD(pParam->dwParam[3]));
								not.xMsg += "]";
								not.dwTimes = 1;
								not.dwColor = 0xffff00ff;
								g_xThreadBuffer.Reset();
								g_xThreadBuffer << not;
								GameSceneManager::GetInstance()->BroadcastPacketAllScene(&g_xThreadBuffer);
							}
							else if(nQualityIndex > 8)
							{
								//	��������
								bItemValid = false;
							}

							//	������������
							SetItemHideAttrib(&pGroundItem->stAttrib);
						}
						else if(pGroundItem->stAttrib.type == ITEM_OTHER &&
							(pGroundItem->stAttrib.curse == kMaterial_Ore ||
							pGroundItem->stAttrib.curse == kMaterial_Wood ||
							pGroundItem->stAttrib.curse == kMaterial_Gem ||
							pGroundItem->stAttrib.curse == kMaterial_Cloth)
							)
						{
							//	��ʯ Ʒ��
							SetMaterialItemQuality(&pGroundItem->stAttrib);
						}
						
						if(bItemValid)
						{
							pScene->InsertGroundItem(pGroundItem);
						}
						else
						{
							SAFE_DELETE(pGroundItem);
						}
					}
				}
			}

			SAFE_DELETE(pItems);
			SAFE_DELETE(pParam);
		}break;
	case DP_DROPITEMLOADED:
		{
			DBOperationParam* pParam = (DBOperationParam*)_dp.uParam0;
			std::list<int>* pItems = (std::list<int>*)pParam->dwParam[2];
			WORD wPosX = LOWORD(pParam->dwParam[1]);
			WORD wPosY = HIWORD(pParam->dwParam[1]);
			size_t nSize = pItems->size();
			GameScene* pScene = GameSceneManager::GetInstance()->GetScene(HIWORD(pParam->dwParam[3]));
			int nPosSize = 0;
			int nPosX = 0;
			int nPosY = 0;
			bool bCanDrop = true;
			int nWhileCounter = 0;

			for(std::list<int>::const_iterator iter = pItems->begin();
				iter != pItems->end();
				++iter)
			{
				bCanDrop = true;
				nWhileCounter = 0;

				nPosX = wPosX;
				nPosY = wPosY;
				nPosX += g_nSearchPoint[nPosSize * 2];
				nPosY += g_nSearchPoint[nPosSize * 2 + 1];
				++nPosSize;
				if(nPosSize >= sizeof(g_nSearchPoint) / 2)
				{
					nPosSize = 0;
				}

				nWhileCounter = 0;
				while(!pScene->CanDrop(nPosX, nPosY))
				{
					nPosX = wPosX;
					nPosY = wPosY;
					nPosX += g_nSearchPoint[nPosSize * 2];
					nPosY += g_nSearchPoint[nPosSize * 2 + 1];
					++nPosSize;
					if(nPosSize >= sizeof(g_nSearchPoint) / 2)
					{
						nPosSize = 0;
					}
					++nWhileCounter;
					if(nWhileCounter >= 25)
					{
						bCanDrop = false;
						break;
					}
				}

				if(bCanDrop)
				{
					int nExtraProb = MAGIC_ITEM_BASE;

					if(pParam->dwParam[4] != 0)
					{
						HeroObject* _pHero = (HeroObject*)pScene->GetPlayerWithoutLock(pParam->dwParam[4]);
						if(_pHero)
						{
							nExtraProb = MAGIC_ITEM_BASE - 1000 * _pHero->GetStateController()->GetMagicItemAddition();
						}
					}

					if(pParam->dwParam[5] != 0)
					{
						GameObject* pMons = pScene->GetNPCByHandleID(pParam->dwParam[5]);
						if(pMons)
						{
							if(pMons->IsElite())
							{
								nExtraProb -= MONS_ELITE_MITEM;
							}
							else if(pMons->IsLeader())
							{
								nExtraProb -= MONS_LEADER_MITEM;
							}
						}
					}
					
					if(nExtraProb < MAGIC_ITEM_MIN)
					{
						nExtraProb = MAGIC_ITEM_MIN;
					}

					pScene->CreateGroundItem(*iter, nPosX, nPosY, nExtraProb);
					//pScene->NewGroundItem(*iter, nPosX, nPosY);
				}
			}
			delete pItems;
			delete pParam;
		}break;
	default:
		{
			LOG(WARNING) << "Unknown world process code[" << _dp.uOp << "]";
		}break;
	}
}

void GameWorld::DoDelaySystem(const DelayedProcess& _dp)
{
	RECORD_FUNCNAME_WORLD;

	switch(_dp.uOp)
	{
	case DP_RELOADSCRIPT:
		{
			GameSceneManager::GetInstance()->ReloadScript();
			if(false == GameWorld::GetInstance().ReloadScript())
			{
				LOG(INFO) << "Can't reload script[world]";
			}
			else
			{
				LOG(INFO) << "Reload script success[world]";
			}

			if(GameWorld::GetInstance().LoadBlackList())
			{
				LOG(INFO) << "Load black list succeed";
			}
			else
			{
				LOG(INFO) << "Can't load the black list...";
			}
			DBOperationParam* pParam = new DBOperationParam;
			pParam->dwOperation = DO_QUERY_RELOADSCRIPT;
			DBThread::GetInstance()->AsynExecute(pParam);
		}break;
	case DP_GETUSERINFO:
		{

		}break;
	case DP_SENDSYSMSG:
		{
			
			const char* pszMsg = (const char*)_dp.uParam0;
			/*			GameSceneManager::GetInstance()->SendRawSystemMessageAllScene(pszMsg);*/
			PkgSystemNotifyNot nreq;
			nreq.xMsg = pszMsg;
			nreq.dwTimes = 1;
			nreq.dwColor = 0xff00BFFF;
			g_xThreadBuffer.Reset();
			g_xThreadBuffer << nreq;
			GameSceneManager::GetInstance()->BroadcastPacketAllScene(&g_xThreadBuffer);

			SAFE_DELETE_ARRAY(pszMsg);
		}break;
	}
}

/************************************************************************/
/* GameObject* GetPlayer(unsigned int _uId)
/************************************************************************/
GameObject* GameWorld::GetPlayer(unsigned int _uId)
{
	RECORD_FUNCNAME_WORLD;

	GameObject* pObj = NULL;

	ObjectMap::const_iterator iter = m_xPlayers.find(_uId);
	if(iter != m_xPlayers.end())
	{
		pObj = iter->second;
	}
	return pObj;
}
/************************************************************************/
/* GameObject* GetNPC(unsigned int _uId)
/************************************************************************/
GameObject* GameWorld::GetNPC(unsigned int _uId)
{
	RECORD_FUNCNAME_WORLD;

	GameObject* pObj = NULL;

	ObjectMap::const_iterator iter = m_xNPC.find(_uId);
	if(iter != m_xNPC.end())
	{
		pObj = iter->second;
	}
	return pObj;
}
/************************************************************************/
/* bool NewPlayer(GameObject* _pObj)
/************************************************************************/
bool GameWorld::NewPlayer(GameObject* _pObj)
{
	RECORD_FUNCNAME_WORLD;

	ObjectMap::iterator iter = m_xPlayers.find(_pObj->GetID());
	if(iter == m_xPlayers.end())
	{
		m_xPlayers.insert(std::make_pair(_pObj->GetID(), _pObj));
		return true;
	}
	return false;
}
/************************************************************************/
/* bool RemovePlayer(unsigned int _uId)
/************************************************************************/
bool GameWorld::RemovePlayer(unsigned int _uId)
{
	RECORD_FUNCNAME_WORLD;

	ObjectMap::iterator iter = m_xPlayers.find(_uId);
	if(iter != m_xPlayers.end())
	{
		delete iter->second;
		m_xPlayers.erase(iter);
		return true;
	}
	return false;
}
/************************************************************************/
/* bool NewNPC(unsigned int _uId)
/************************************************************************/
bool GameWorld::NewNPC(unsigned int _uId)
{
	return false;
}
/************************************************************************/
/* void RemoveNPC(unsigned int _uId)
/************************************************************************/
void GameWorld::RemoveNPC(unsigned int _uId)
{
	ObjectMap::iterator iter = m_xNPC.find(_uId);
	if(iter != m_xNPC.end())
	{
		m_xNPC.erase(iter);
	}
}

/************************************************************************/
/* void AddDelayedProcess(DelayedProcess* _pDp)
/************************************************************************/
void GameWorld::AddDelayedProcess(const DelayedProcess* _pDp)
{
	RECORD_FUNCNAME_WORLD;

	try
	{
		LockProcess();
		if(m_xProcessD.GetAvailableLength() < sizeof(DelayedProcess))
		{
			DEBUG_BREAK;
			m_xProcessD.GetAvailableLength();
		}
		m_xProcessD << *_pDp;
		UnlockProcess();
	}
	catch(std::exception& e)
	{
		ALERT_MSGBOX("Delayed process buffer error!");
	}
}
//////////////////////////////////////////////////////////////////////////
bool GameWorld::LoadMagicInfo()
{
	//RECORD_FUNCNAME_WORLD;

	MagicInfo* pInfo = NULL;
	char szPath[MAX_PATH];
	sprintf(szPath, "%s\\Config\\Magica.ini",
		GetRootPath());
	bool bExist = false;
	if(0 == _access(szPath, 0))
	{
		bExist = true;
	}

	DWORD dwValue = 0;
	char szKey[10];

	for(int i = 0; i < MEFF_USERTOTAL; ++i)
	{
		pInfo = &g_xMagicInfoTable[i];
		ZeroMemory(pInfo, sizeof(MagicInfo));
		pInfo->wID = i;
		itoa(i, szKey, 10);

		if(bExist)
		{
			dwValue = GetPrivateProfileInt(szKey, "ID", 0, szPath);
			if(dwValue != 0)
			{
				dwValue = GetPrivateProfileInt(szKey, "LEVEL1", 0, szPath);
				pInfo->wLevel[0] = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "LEVEL2", 0, szPath);
				pInfo->wLevel[1] = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "LEVEL3", 0, szPath);
				pInfo->wLevel[2] = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "LEVEL4", 0, szPath);
				pInfo->wLevel[3] = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "LEVEL5", 0, szPath);
				pInfo->wLevel[4] = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "LEVEL6", 0, szPath);
				pInfo->wLevel[5] = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "LEVEL7", 0, szPath);
				pInfo->wLevel[6] = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "JOB", 0, szPath);
				pInfo->bJob = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "MULTIPLE", 0, szPath);
				pInfo->bMultiple = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "INCREASE", 0, szPath);
				pInfo->wIncrease = dwValue;
				dwValue = GetPrivateProfileInt(szKey, "BASECOST", 0, szPath);
				pInfo->bBaseCost = dwValue;
			}
		}
	}

	return bExist;
}
//////////////////////////////////////////////////////////////////////////
void GameWorld::UpgradeItems(ItemAttrib* _pItem, int _nProb)
{
	RECORD_FUNCNAME_WORLD;

#ifdef _DEBUG
	if(_pItem->type == ITEM_WEAPON ||
		_pItem->type == ITEM_RING ||
		_pItem->type == ITEM_BRACELAT ||
		_pItem->type == ITEM_CLOTH)
	{
		//DEBUG_BREAK;
	}
#endif

	//const int nProp = 35000;
	int nProp = _nProb;
	if(nProp < MAGIC_ITEM_MIN)
	{
		nProp = MAGIC_ITEM_MIN;
	}

	bool bCanUp = false;
	static int nCrash = 1000;
	if(nCrash > /*60000*/nProp/* + 5000*/)
	{
		nCrash = 1000;
	}
	int nRandom = rand() % /*50000*/nProp;
	if(nRandom <= nCrash &&
		nRandom >= (nCrash - 1000))
	{
		bCanUp = true;
	}

	if(!bCanUp)
	{
		return;
	}

	int nTotalUp = 0;
	nRandom -= (nCrash - 1000);

	if(nRandom >= 0 &&
		nRandom <= 0)
	{
		nTotalUp = 7;
	}
	else if(nRandom >= 1 &&
		nRandom <= 3)
	{
		nTotalUp = 6;
	}
	else if(nRandom >= 4 &&
		nRandom <= 8)
	{
		nTotalUp = 5;
	}
	else if(nRandom >= 9 &&
		nRandom <= 14)
	{
		nTotalUp = 4;
	}
	else if(nRandom >= 15 &&
		nRandom <= 80)
	{
		nTotalUp = 3;
	}
	else if(nRandom >= 81 &&
		nRandom <= 250)
	{
		nTotalUp = 2;
	}
	else
	{
		nTotalUp = 1;
	}

	/*int nLoopCounter = 0;
	int nSize = 0;
	int nIndex = 0;
	int nUp = 0;
	bool bOk = false;

	if(_pItem->type == ITEM_WEAPON)
	{
		nSize = sizeof(s_nWeaponUpAttrib) / sizeof(s_nWeaponUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nWeaponUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_CLOTH)
	{
		nSize = sizeof(s_nClothUpAttrib) / sizeof(s_nClothUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nClothUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_RING)
	{
		nSize = sizeof(s_nRingUpAttrib) / sizeof(s_nRingUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nRingUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_HELMET)
	{
		nSize = sizeof(s_nHelmetUpAttrib) / sizeof(s_nHelmetUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nHelmetUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_BRACELAT)
	{
		nSize = sizeof(s_nBraUpAttrib) / sizeof(s_nBraUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nBraUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_NECKLACE)
	{
		nSize = sizeof(s_nNeckUpAttrib) / sizeof(s_nNeckUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nNeckUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			if(nIndex == ATTRIB_LUCKY &&
				_pItem->lucky + nUp > 3)
			{
				continue;
			}

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_SHOE)
	{
		nSize = sizeof(s_nShoeUpAttrib) / sizeof(s_nShoeUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nShoeUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_BELT)
	{
		nSize = sizeof(s_nBeltUpAttrib) / sizeof(s_nBeltUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nBeltUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_GEM)
	{
		nSize = sizeof(s_nGemUpAttrib) / sizeof(s_nGemUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nGemUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}*/
	UpgradeItemsWithAddition(_pItem, nTotalUp);

	nCrash += 1000;
}
//////////////////////////////////////////////////////////////////////////
void GameWorld::UpgradeItems(ItemAttrib* _pItem)
{
	RECORD_FUNCNAME_WORLD;

	static const int s_nWeaponUpAttrib[] =
	{
		ATTRIB_DC,
		ATTRIB_DC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_MC,
		ATTRIB_MC,
		ATTRIB_SC,
		ATTRIB_SC,
		ATTRIB_SC,
		//ATTRIB_LUCKY,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU
	};

	static const int s_nRingUpAttrib[] =
	{
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nBraUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nNeckUpAttrib[] =
	{
		ATTRIB_DC,
		ATTRIB_DC,
		ATTRIB_DC,
		ATTRIB_SC,
		ATTRIB_SC,
		ATTRIB_SC,
		ATTRIB_MC,
		ATTRIB_MC,
		ATTRIB_MC,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_ACCU,
		ATTRIB_HIDE,
		ATTRIB_LUCKY
	};

	static const int s_nClothUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_SC,
		ATTRIB_MC
	};

	static const int s_nHelmetUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nShoeUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

	static const int s_nBeltUpAttrib[] =
	{
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_AC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_MAC,
		ATTRIB_DC,
		ATTRIB_MC,
		ATTRIB_SC
	};

#ifdef _DEBUG
	if(_pItem->type == ITEM_WEAPON ||
		_pItem->type == ITEM_RING ||
		_pItem->type == ITEM_BRACELAT ||
		_pItem->type == ITEM_CLOTH)
	{
		//DEBUG_BREAK;
	}
#endif

	const int nProp = 35000;

	bool bCanUp = false;
	static int nCrash = 1000;
	if(nCrash > /*60000*/nProp + 5000)
	{
		nCrash = 1000;
	}
	int nRandom = rand() % /*50000*/nProp;
	if(nRandom <= nCrash &&
		nRandom >= (nCrash - 1000))
	{
		bCanUp = true;
	}

	if(!bCanUp)
	{
		return;
	}

	int nTotalUp = 0;
	nRandom -= (nCrash - 1000);

	if(nRandom >= 0 &&
		nRandom <= 0)
	{
		nTotalUp = 7;
	}
	else if(nRandom >= 1 &&
		nRandom <= 3)
	{
		nTotalUp = 6;
	}
	else if(nRandom >= 4 &&
		nRandom <= 8)
	{
		nTotalUp = 5;
	}
	else if(nRandom >= 9 &&
		nRandom <= 14)
	{
		nTotalUp = 4;
	}
	else if(nRandom >= 15 &&
		nRandom <= 80)
	{
		nTotalUp = 3;
	}
	else if(nRandom >= 81 &&
		nRandom <= 250)
	{
		nTotalUp = 2;
	}
	else
	{
		nTotalUp = 1;
	}

	int nLoopCounter = 0;
	int nSize = 0;
	int nIndex = 0;
	int nUp = 0;
	bool bOk = false;

	if(_pItem->type == ITEM_WEAPON)
	{
		nSize = sizeof(s_nWeaponUpAttrib) / sizeof(s_nWeaponUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nWeaponUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_CLOTH)
	{
		nSize = sizeof(s_nClothUpAttrib) / sizeof(s_nClothUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nClothUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_RING)
	{
		nSize = sizeof(s_nRingUpAttrib) / sizeof(s_nRingUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nRingUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_HELMET)
	{
		nSize = sizeof(s_nHelmetUpAttrib) / sizeof(s_nHelmetUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nHelmetUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_BRACELAT)
	{
		nSize = sizeof(s_nBraUpAttrib) / sizeof(s_nBraUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nBraUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_NECKLACE)
	{
		nSize = sizeof(s_nNeckUpAttrib) / sizeof(s_nNeckUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nNeckUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			if(nIndex == ATTRIB_LUCKY &&
				_pItem->lucky + nUp > 3)
			{
				continue;
			}

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_SHOE)
	{
		nSize = sizeof(s_nShoeUpAttrib) / sizeof(s_nShoeUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nShoeUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_BELT)
	{
		nSize = sizeof(s_nBeltUpAttrib) / sizeof(s_nBeltUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nBeltUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}

	nCrash += 1000;
}

bool GameWorld::UpgradeAttrib(ItemAttrib* _pItem, int _index, int _value)
{
	RECORD_FUNCNAME_WORLD;

	bool bOk = false;

	if(_index == ATTRIB_DC)
	{
		_pItem->maxDC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_MC)
	{
		_pItem->maxMC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_SC)
	{
		_pItem->maxSC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_AC)
	{
		_pItem->maxAC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_MAC)
	{
		_pItem->maxMAC += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_LUCKY)
	{
		_pItem->lucky += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_ACCU)
	{
		_pItem->accuracy += _value;
		bOk = true;
	}
	else if(_index == ATTRIB_HIDE)
	{
		_pItem->hide += _value;
		bOk = true;
	}

	if(bOk)
	{
		int nPreAdd = 0;
		int nValue = 0;

		if(_pItem->level != 0)
		{
			BYTE bLow = LOBYTE(_pItem->level);
			BYTE bHigh = HIBYTE(_pItem->level);
			BYTE bKey = GetItemMakeMask(bHigh);
			nValue = GetMakeMaskValue(bKey);
			nPreAdd = bLow - nValue;
		}
		nPreAdd += _value;

		int nRandom = 1 + rand() % 254;
		nValue = GetItemMakeMask(nRandom);
		nValue = GetMakeMaskValue(nValue);
		nPreAdd += nValue;
		_pItem->level = MAKEWORD(nPreAdd, nRandom);
	}
	return bOk;
}

bool GameWorld::LoadScript()
{
	m_xScript.Destroy();
	m_xScript.Create();
	tolua_BackMirServer_open(m_xScript.GetVM());

	//	ѹ��ű�����
	lua_State* pState = m_xScript.GetVM();
	//tolua_pushusertype(pState, &m_xScript, "LuaServerEngine");
	//lua_setglobal(pState, "ENGINE");
	m_xScript.PushEngine();

	bool bRet = m_xScript.DoModule("server/world/world");

	/*if(bRet)
	{
		//	��ȡ�̳��б�
		lua_getglobal(pState, "InitOlShopList");
		if(lua_isfunction(pState, -1))
		{
			tolua_pushusertype(pState, OlShopManager::GetInstance(), "OlShopManager");
			int nRet = lua_pcall(pState, 1, 0, 0);
			if(nRet != 0)
			{
				LOG(WARNING) << lua_tostring(pState, -1);
				lua_pop(pState, 1);
			}
		}
	}*/

	return bRet;
}

bool GameWorld::ReloadScript()
{
	return LoadScript();

	/*m_xScript.Destroy();
	m_xScript.Create();
	char szBuf[MAX_PATH];
#ifdef _DEBUG
	sprintf(szBuf, "%s\\Help\\world.lua",
		GetRootPath());

	tolua_BackMirServer_open(m_xScript.GetVM());

	if(m_xScript.LoadFile(szBuf))
	{
		return true;
	}
	return false;
#else
	char szFile[50];
#ifdef _LUAJIT_
	strcpy(szFile, "world.bjt");
#else
	strcpy(szFile, "world.bbt");
#endif
	sprintf(szBuf, "%s\\Help\\dog.idx",
		GetRootPath());

	if(m_xScript.ExecuteZip(szBuf, szFile))
	{
		return true;
	}
	return false;
#endif*/
}

void GameWorld::UpgradeItemsWithAddition(ItemAttrib* _pItem, int _nAddition)
{
	int nLoopCounter = 0;
	int nSize = 0;
	int nIndex = 0;
	int nUp = 0;
	bool bOk = false;
	int nTotalUp = _nAddition;

	if(_pItem->type == ITEM_WEAPON)
	{
		nSize = sizeof(s_nWeaponUpAttrib) / sizeof(s_nWeaponUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nWeaponUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_CLOTH)
	{
		nSize = sizeof(s_nClothUpAttrib) / sizeof(s_nClothUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nClothUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_RING)
	{
		nSize = sizeof(s_nRingUpAttrib) / sizeof(s_nRingUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nRingUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_HELMET)
	{
		nSize = sizeof(s_nHelmetUpAttrib) / sizeof(s_nHelmetUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nHelmetUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_BRACELAT)
	{
		nSize = sizeof(s_nBraUpAttrib) / sizeof(s_nBraUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nBraUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_NECKLACE)
	{
		nSize = sizeof(s_nNeckUpAttrib) / sizeof(s_nNeckUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nNeckUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			if(nIndex == ATTRIB_LUCKY &&
				_pItem->lucky + nUp > 3)
			{
				continue;
			}

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_SHOE)
	{
		nSize = sizeof(s_nShoeUpAttrib) / sizeof(s_nShoeUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nShoeUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_BELT)
	{
		nSize = sizeof(s_nBeltUpAttrib) / sizeof(s_nBeltUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nBeltUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_GEM)
	{
		nSize = sizeof(s_nGemUpAttrib) / sizeof(s_nGemUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nGemUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
	else if(_pItem->type == ITEM_MEDAL)
	{
		nSize = sizeof(s_nMedalUpAttrib) / sizeof(s_nMedalUpAttrib[0]);
		while(nTotalUp > 0)
		{
			++nLoopCounter;
			if(nLoopCounter > 100)
			{
				break;
			}

			bOk = false;
			nIndex = rand() % nSize;
			nIndex = s_nMedalUpAttrib[nIndex];
			nUp = rand() % nTotalUp + 1;

			bOk = UpgradeAttrib(_pItem, nIndex, nUp);
			if(bOk)
			{
				nTotalUp -= nUp;
			}
		}
	}
}

bool GameWorld::LoadBlackList()
{
	char* pszFileData = NULL;
	size_t uDataSize = 0;
	char szBuf[MAX_PATH];
	sprintf(szBuf, "%s\\blacklist.txt",
		GetRootPath());

	HANDLE hFile = ::CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return false;
	}
	size_t uFileSize = GetFileSize(hFile, NULL);

	if(0 != uFileSize)
	{
		uDataSize = uFileSize + 1;
		pszFileData = new char[uDataSize];
		pszFileData[uDataSize - 1] = 0;
	}
	DWORD dwRead = 0;
	if(FALSE == ReadFile(hFile, pszFileData, uFileSize, &dwRead, NULL))
	{
		CloseHandle(hFile);
		hFile = 0;
		delete[] pszFileData;
		pszFileData = NULL;
	}
	CloseHandle(hFile);

	if(NULL == pszFileData)
	{
		return false;
	}

	std::istringstream is(pszFileData);
	pszFileData[0] = 0;
	m_xBlackList.clear();

	BlackListItem item;
	char szName[20];
	char szJob[20];
	char szSex[20];
	int nLevel = 0;

	while(is.getline(pszFileData, uDataSize))
	{
		item.xName.clear();
		item.bJob = 0xFF;
		item.bSex = 0xFF;
		item.nLevel = -1;
		if(pszFileData[0] == ';')
		{
			continue;
		}

		if(4 == sscanf(pszFileData, "%s %s %s %d",
			szName, szJob, szSex, &nLevel))
		{
			if(0 == strcmp(szJob, "սʿ"))
			{
				item.bJob = 0;
			}
			else if(0 == strcmp(szJob, "��ʦ"))
			{
				item.bJob = 1;
			}
			else if(0 == strcmp(szJob, "��ʿ"))
			{
				item.bJob = 2;
			}

			if(0 == strcmp(szSex, "��"))
			{
				item.bSex = 1;
			}
			else if(0 == strcmp(szSex, "Ů"))
			{
				item.bSex = 2;
			}

			item.xName = szName;
			item.nLevel = nLevel;

			if(!item.xName.empty() &&
				item.bJob != 0xFF &&
				item.bSex != 0xFF &&
				item.nLevel != -1)
			{
				m_xBlackList.push_back(item);
			}
		}
		pszFileData[0] = 0;
	}

	delete[] pszFileData;
	pszFileData = NULL;

	return true;
}

bool GameWorld::IsInBlackList(HeroObject* _pHero)
{
	if(NULL == _pHero)
	{
		return false;
	}

	BlackList::const_iterator begIter = m_xBlackList.begin();
	BlackList::const_iterator endIter = m_xBlackList.end();

	for(begIter;
		begIter != endIter;
		++begIter)
	{
		if(_pHero->IsEncrypt())
		{
			char szName[20];
			ObjectValid::GetItemName(&_pHero->GetUserData()->stAttrib, szName);

			if(begIter->xName == szName &&
				begIter->nLevel == _pHero->GetLevel() &&
				begIter->bJob == _pHero->GetUserData()->bJob &&
				begIter->bSex == _pHero->GetHeroSex())
			{
				return true;
			}
		}
		else
		{
			if(begIter->xName == _pHero->GetUserData()->stAttrib.name &&
				begIter->nLevel == _pHero->GetLevel() &&
				begIter->bJob == _pHero->GetUserData()->bJob &&
				begIter->bSex == _pHero->GetHeroSex())
			{
				return true;
			}
		}
	}

	return false;
}

unsigned int GameWorld::Thread_ProcessMessage(const MSG* _pMsg)
{
	unsigned int uRet = 0;
	DWORD dwCurrentTick = GetTickCount();

	if(_pMsg->message != WM_WORLDCHECKCRC)
	{
		g_xConsole.CPrint("Receive thread msg:%d", _pMsg->message);
	}

	static int s_nTimeoutCounter = 0;

	if(_pMsg->message == WM_WORLDCHECKCRC)
	{
		if(dwCurrentTick - _pMsg->wParam >= 0 &&
			dwCurrentTick - _pMsg->wParam <= 500)
		{
			//	Check crc
			WORD wCRC16 = 0;
			CalcCRC16(&wCRC16, 4, (LPBYTE)&_pMsg->wParam);

			if(wCRC16 != _pMsg->lParam)
			{
#ifndef _DISABLE_CONSOLE
				g_xConsole.CPrint("A invalid CRC16 data.");
#endif
				if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
				{
					LOG(ERROR) << "DESTORY_GAMEWORLD";
				}
				else
				{
					DESTORY_GAMEWORLD;
				}
			}
			else
			{
				m_dwLastRecWatcherMsgTime = dwCurrentTick;
			}
		}
		else
		{
#ifndef _DISABLE_CONSOLE
			g_xConsole.CPrint("Invalid thread message from WatcherThread.Current tick[%d] and sent tick[%d]",
				GetTickCount(), _pMsg->wParam);
#endif
			++s_nTimeoutCounter;

			if(s_nTimeoutCounter > 5)
			{
				if(CMainServer::GetInstance()->GetServerMode() == GM_LOGIN)
				{
					LOG(ERROR) << "DESTORY_GAMEWORLD";
				}
				else
				{
					DESTORY_GAMEWORLD;
				}
			}
		}
	}
	else if(_pMsg->message == WM_USERCONNECTED)
	{
		/*PkgLoginGameTypeNot not;
		not.bType = PLGTN_GAMESERVER;
		not.dwConnIdx = _pMsg->wParam;
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << not;
		SendBuffer(_pMsg->wParam, &g_xThreadBuffer);*/
	}
	else if(_pMsg->message == WM_PLAYERLOGIN)
	{
		LoginQueryInfo* pInfo = (LoginQueryInfo*)_pMsg->lParam;
		HANDLE hEvent = (HANDLE)_pMsg->wParam;

		HeroObject* pHero = (HeroObject*)GameSceneManager::GetInstance()->GetPlayerByName(pInfo->szName);
		pInfo->bExists = (pHero != NULL);
		if(NULL != pHero)
		{
			pInfo->dwConnID = pHero->GetUserIndex();
		}

		SetEvent(hEvent);
	}
	else if(_pMsg->message == WM_PLAYERRANKLIST)
	{
		SAFE_DELETE_ARRAY(m_pRankListData);
		m_pRankListData = (char*)_pMsg->wParam;
	}
	else if(_pMsg->message == WM_CHECKBUYOLSHOPITEM)
	{
		PkgLoginCheckBuyShopItemAck* pAck = (PkgLoginCheckBuyShopItemAck*)_pMsg->wParam;
		if(NULL != pAck)
		{
			//	������ͬUID���̳ǹ���
			//	�ȼ��queryid
			int nQueryId = OlShopManager::GetInstance()->GetQueryID(pAck->nUid);

			if(nQueryId == pAck->nQueryId)
			{
				OlShopManager::GetInstance()->SetWaitForQuery(pAck->nUid, false);

				HeroObject* pHero = (HeroObject*)GameSceneManager::GetInstance()->GetPlayer(pAck->nGsId);
				if(NULL != pHero)
				{
					pHero->DoPacket(*pAck);
				}
			}
			
			SAFE_DELETE(pAck);
		}
	}
	else if(_pMsg->message == WM_CONSUMEDONATE)
	{
		PkgLoginConsumeDonateAck* pAck = (PkgLoginConsumeDonateAck*)_pMsg->wParam;
		if(NULL != pAck)
		{
			HeroObject* pHero = (HeroObject*)GameSceneManager::GetInstance()->GetPlayer(pAck->nGsId);
			if(NULL != pHero)
			{
				pHero->DoPacket(*pAck);
			}
			SAFE_DELETE(pAck);
		}
	}
	else if(_pMsg->message == WM_LSCONNECTED)
	{
		m_xScript.DispatchEvent(kLuaEvent_WorldLoginServerConnected, NULL);
	}
	else if(_pMsg->message == WM_SCHEDULEACTIVE)
	{
		LuaEvent_WorldScheduleActive lw;
		lw.nScheduleId = _pMsg->wParam;
		m_xScript.DispatchEvent(kLuaEvent_WorldScheduleActive, &lw);
	}
	else if(_pMsg->message == WM_SETDIFFICULTY)
	{
		int nDiff = _pMsg->wParam;

		if(nDiff < kDifficultyDefault ||
			nDiff >= kDifficultyTotal ||
			CMainServer::GetInstance()->GetServerMode() == GM_LOGIN ||
			kDifficultyDefault != GameWorld::GetInstance().GetDifficultyLevel())
		{
			GameSceneManager::GetInstance()->SendSystemMessageAllScene("�޷������Ѷ�����");
		}
		else
		{
			GameWorld::GetInstance().SetDifficultyLevel(nDiff);
			GameSceneManager::GetInstance()->SendSystemMessageAllScene("�����Ѷ����óɹ�");
		}
	}

	return uRet;
}

static int s_nHideAttribArray[] = {
	HideAttrib_AC,
	HideAttrib_AC,
	HideAttrib_AC,
	HideAttrib_AC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_MagicDrop,
	HideAttrib_MainAttrib,
	HideAttrib_Lucky,
	HideAttrib_DcHide,
	HideAttrib_MagicHide
};

//	����
static int s_nDefenceEquipArray[] = {
	HideAttrib_AC,
	HideAttrib_AC,
	HideAttrib_AC,
	HideAttrib_AC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_MagicDrop
};

//	����
static int s_nAttackEquipArray[] = {
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_MagicDrop,
	HideAttrib_Lucky
};

//	����
static int s_nJewelryEquipArray[] = {
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_MAC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_DC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_MC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_SC,
	HideAttrib_MagicDrop,
	HideAttrib_DcHide,
	HideAttrib_MagicHide
};

static bool GetHideAttribType(int _nItemType, int _nTypeSum, int _nTypes[])
{
	for(int i = 0; i < 3; ++i)
	{
		_nTypes[i] = HideAttrib_None;
	}

	int nSourceArraySize = 0;
	int* pSourceArray = NULL;

	if(HeroObject::IsAttackItem(_nItemType))
	{
		pSourceArray = s_nAttackEquipArray;
		nSourceArraySize = sizeof(s_nAttackEquipArray) / sizeof(s_nAttackEquipArray[0]);
	}
	else if(HeroObject::IsDefenceItem(_nItemType))
	{
		pSourceArray = s_nDefenceEquipArray;
		nSourceArraySize = sizeof(s_nDefenceEquipArray) / sizeof(s_nDefenceEquipArray[0]);
	}
	else if(HeroObject::IsJewelryItem(_nItemType))
	{
		pSourceArray = s_nJewelryEquipArray;
		nSourceArraySize = sizeof(s_nJewelryEquipArray) / sizeof(s_nJewelryEquipArray[0]);
	}

	if(NULL == pSourceArray)
	{
		return false;
	}

	for(int i = 0; i < _nTypeSum; ++i)
	{
		int nAttrib = HideAttrib_None;

		while(1)
		{
			int nRandom = rand() % nSourceArraySize;
			nAttrib = pSourceArray[nRandom];

			bool bAttribExists = false;
			for(int j = 0; j < 3; ++j)
			{
				if(_nTypes[j] == nAttrib)
				{
					bAttribExists = true;
					break;
				}
			}

			if(!bAttribExists)
			{
				break;
			}
		}

		_nTypes[i] = nAttrib;
	}

	return true;
}

static int GetHideAttribValue(int _nType)
{
	int nValue = 0;

	switch(_nType)
	{
	case HideAttrib_AC:
	case HideAttrib_MAC:
		{
			//	1-4
			nValue = rand() % 4 + 1;
			//	1-8
			if(rand() % 5 == 0)
			{
				nValue += rand() % 5;
			}
			//	1-12
			if(rand() % 20 == 0)
			{
				nValue += rand() % 5;
			}

			int nRand = rand() % 20;
			if(nRand <= 10)
			{
				nValue += rand() % 4;
			}
			else if(nRand <= 17)
			{
				nValue += 4;
				nValue += rand() % 4;
			}
			else
			{
				nValue += 8;
				nValue += rand() % 6;
			}
		}break;
	case HideAttrib_DC:
	case HideAttrib_MC:
	case HideAttrib_SC:
		{
			//	1-4
			nValue = rand() % 4 + 1;
			//	1-8
			if(rand() % 8 == 0)
			{
				nValue += rand() % 5;
			}
			//	1-12
			if(rand() % 20 == 0)
			{
				nValue += rand() % 5;
			}

			nValue = 1;

			int nRand = rand() % 30;
			if(nRand <= 20)
			{
				nValue += rand() % 4;
			}
			else if(nRand <= 28)
			{
				nValue += 4;
				nValue += rand() % 4;
			}
			else
			{
				nValue += 8;
				nValue += rand() % 6;
			}
		}break;
	case HideAttrib_DcHide:
	case HideAttrib_MagicHide:
		{
			nValue = rand() % 2 + 1;
		}break;
	case HideAttrib_MagicDrop:
	case HideAttrib_MainAttrib:
	case HideAttrib_Lucky:
		{
			nValue = rand() % 2 + 1;
		}break;
	}

	return nValue;
}

void GameWorld::SetItemHideAttrib(ItemAttrib *_pItem)
{
	if(!HeroObject::IsEquipItem(_pItem->type))
	{
		return;
	}

	//	��װ�����Ӽ�������
	int nGiveHideAttribNumber = rand() % 40;
	
	if(nGiveHideAttribNumber != 0)
	{
#ifdef _DEBUG
#else
		return;
#endif
	}

	int nAttribSumNumber = rand() % 75;
	int nAttribSum = 1;

	if(nAttribSumNumber <= 1)
	{
		nAttribSum = 3;
	}
	else if(nAttribSumNumber <= 12)
	{
		nAttribSum = 2;
	}

#ifdef _DEBUG
	nAttribSum = 3;
#endif

	UINT uAttribCode = 0;
	HideAttribHelper::SetAllAttribCount(uAttribCode, nAttribSum);

	int nTypes[3] = {0};
	if(GetHideAttribType(_pItem->type, nAttribSum, nTypes))
	{
		for(int i = 0; i < nAttribSum; ++i)
		{
			int nAttribType = nTypes[i];
			int nAttribValue = GetHideAttribValue(nAttribType);

			if(nAttribValue == 0)
			{
				return;
			}

			HideAttribHelper::SetActiveAttribType(uAttribCode, i, nTypes[i]);
			HideAttribHelper::SetActiveAttribValue(uAttribCode, i, nAttribValue);
		}
	}
	else
	{
		uAttribCode = 0;
	}

	_pItem->maxMP = uAttribCode;
}

void GameWorld::SetMaterialItemQuality(ItemAttrib* _pItem)
{
	if(_pItem->type != ITEM_OTHER)
	{
		return;
	}
	if(_pItem->curse != kMaterial_Ore &&
		_pItem->curse != kMaterial_Wood &&
		_pItem->curse != kMaterial_Cloth &&
		_pItem->curse != kMaterial_Gem)
	{
		return;
	}

	_pItem->lucky = 5;

	int nRandom = rand() % 100;
	if(nRandom < 80)
	{
		_pItem->lucky += rand() % 5;
	}
	else if(nRandom < 95)
	{
		_pItem->lucky += rand() % 10;
	}
	else
	{
		_pItem->lucky += rand() % 15;
	}
}

void GameWorld::SetSchedule(int _nEventId, const char* _pszCronExpr)
{
	/*
	//	Now can directly use cron in c++
	PkgLoginRegisterScheduleReq req;
	req.nEventId = _nEventId;
	req.xCronExpr = _pszCronExpr;

	DWORD dwLsIndex = CMainServer::GetInstance()->GetLSConnIndex();
	if(0 != dwLsIndex)
	{
		g_xThreadBuffer.Reset();
		g_xThreadBuffer << req;
		SendBufferToServer(dwLsIndex, &g_xThreadBuffer);
	}*/
	m_xCronScheduler.AddCronJob(_nEventId, _pszCronExpr, &GameWorld::__cronActive, 0);
}

void GameWorld::ResetSchedule(int _nEventId)
{
	m_xCronScheduler.RemoveCronJob(_nEventId);
}

int GameWorld::__cronActive(int _nJobId, int _nArg)
{
	LuaEvent_WorldScheduleActive lw;
	lw.nScheduleId = _nJobId;
	GameWorld::GetInstance().m_xScript.DispatchEvent(kLuaEvent_WorldScheduleActive, &lw);

	return 0;
}