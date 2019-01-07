#ifndef CMAINSERVER_H_
#define CMAINSERVER_H_

#include "../../CommonModule/ByteBuffer.h"
#include "../../CommonModule/GamePacket.h"
#include "../GameWorld/WatcherThread.h"
#include "../common/shared.h"
#include <string>
#include <set>
#include <mutex>

struct ServerState;
struct PacketBase;
struct PacketHeader;
class GameObject;
//////////////////////////////////////////////////////////////////////////
typedef std::map<DWORD, DWORD> Index2UserIDMap;
//////////////////////////////////////////////////////////////////////////
enum {
	MODE_STOP = 0,
	MODE_RUNNING = 1,
};

enum NetThreadEventType
{
	kNetThreadEvent_None = 0,
	kNetThreadEvent_SmallQuit,
};

struct NetThreadEvent
{
	int nEventId;
	int nArg;
};
typedef std::list<NetThreadEvent> NetThreadEventList;
//////////////////////////////////////////////////////////////////////////

namespace ioserver {
	class IOServer;
}

class HeroObject;
class ServerShell;
class LoginExtendInfoParser;
namespace google {
	namespace protobuf {
		class Message;
	}
}

class CMainServer
{
public:
	CMainServer();
	virtual ~CMainServer();

public:
	static CMainServer* GetInstance()
	{
		static CMainServer s_xServer;
		
		return &s_xServer;
	}

public:
	void SetServerShell(ServerShell *_pServerShell);
	ServerShell* GetServerShell();
	bool InitNetWork();
	bool StartServer(char* _szIP, WORD _wPort);
	void StopServer();
	void StopEngine();
	void WaitForStopEngine();
	bool InitDatabase();
	bool InitCRCThread();

	ioserver::IOServer* GetIOServer();

	inline GAME_MODE GetServerMode()						{return m_eMode;}
	inline void SetServerMode(GAME_MODE _eMode)				{m_eMode = _eMode;}
	inline void SetLoginAddr(const std::string& _xLoginAddr)		{m_xLoginAddr = _xLoginAddr;}
	inline DWORD GetLSConnIndex()							{return m_dwLsConnIndex;}

	inline void IncOnlineUsers()							{++m_dwUserNumber;}
	inline void DecOnlineUsers()							{if(m_dwUserNumber > 0) --m_dwUserNumber;}

	inline void SetAppException()							{m_bAppException = true;}
	inline bool GetAppException()							{return m_bAppException;}

	inline bool IsLoginServerMode()							{return m_eMode == GM_LOGIN;}

	void UpdateServerState();

	void ForceCloseConnection(DWORD _dwIndex);

	inline WORD GetListenPort()								{return m_dwListenPort;}
	inline std::string& GetListenIP()						{return m_xListenIP;}

	void SendNetThreadEvent(const NetThreadEvent& _refEvt);
	void ProcessNetThreadEvent();

public:
	inline void SetRunningMode(BYTE _bMode)					{m_bMode = _bMode;}
	inline BYTE GetRunningMode()							{return m_bMode;}
	inline DWORD GetMainThreadID()							{return m_dwThreadID;} 

public:
	static unsigned int SendBuffer(unsigned int _nIdx, ByteBuffer* _pBuf);
	static unsigned int SendBufferToServer(unsigned int _nIdx, ByteBuffer* _pBuf);
	static unsigned int SendProtoToServer(unsigned int _nIdx, int _nCode, google::protobuf::Message& _refMsg);

public:
	void OnAcceptUser(DWORD _dwIndex);
	void OnDisconnectUser(DWORD _dwIndex);
	void OnRecvFromUserTCP(DWORD _dwIndex, ByteBuffer* _xBuf);
	void OnRecvFromServerTCP(DWORD _dwIndex, ByteBuffer* _xBuf);
	void OnRecvFromServerTCPProtobuf(DWORD _dwIndex, ByteBuffer* _xBuf);

protected:
	//	玩家连接
	static void STDCALL _OnAcceptUser(DWORD _dwIndex);
	//	玩家离开
	static void STDCALL _OnDisconnectUser(DWORD _dwIndex);
	//	玩家发送数据包
	static void STDCALL _OnRecvFromUserTCP(DWORD _dwIndex, char* _pMsg, DWORD _dwLen);
	//	登陆服务器连接成功
	static void STDCALL _OnLsConnSuccess(DWORD _dwIndex, void* _pParam);
	static void STDCALL _OnLsConnFailed(DWORD _dwIndex, void* _pParam);
	static void STDCALL _OnRecvFromServerTCP(DWORD _dwIndex, char* _pMsg, DWORD _dwLen);
	static void STDCALL _OnAcceptServer(DWORD _dwIndex);
	static void STDCALL _OnDisconnectServer(DWORD _dwIndex);
	//	循环时间
	static void STDCALL _OnGameLoop(DWORD _dwEvtIndex);

private:
	bool InitLogFile();
	void MakeServerState(ServerState* _pState);
	bool CheckUserValid(GameObject* _pObj);
	bool ConnectToLoginSvr();

	bool CreateLoginHero(HeroObject* _pHero,
		HeroHeader& _refHeroHeader,
		std::vector<char>& _refLoginData,
		LoginExtendInfoParser& _refLoginExt,
		std::string _refErrMsg);
	bool LoadHumData(HeroObject *_pHero, ByteBuffer& _xBuf, USHORT _uVersion);
	bool OnPlayerRequestLogin(DWORD _dwIndex, DWORD _dwLSIndex, DWORD _dwUID, const char* _pExtendInfo, PkgUserLoginReq& req);

	void AddInformationToMessageBoard(const char* fmt, ...);

	// Deprecated
	bool OnPreProcessPacket(DWORD _dwIndex, DWORD _dwLSIndex, DWORD _dwUID, const char* _pExtendInfo, PkgUserLoginReq& req);
	bool LoadHumData110(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData111(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData112(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData113(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData114(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData115(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData116(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData117(HeroObject* _pHero, ByteBuffer& _xBuf);

	bool LoadHumData200(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData201(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData202(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData203(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData204(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData205(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData206(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData207(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData208(HeroObject* _pHero, ByteBuffer& _xBuf);
	bool LoadHumData210(HeroObject* _pHero, ByteBuffer& _xBuf);

protected:
	ioserver::IOServer *m_pIOServer;

	//	服务器运行模式
	BYTE m_bMode;
	DWORD m_dwUserNumber;

	DWORD m_dwThreadID;

	//	CRC检测线程
	WatcherThread* m_pWatcherThread;

	//	服务器游戏模式
	GAME_MODE m_eMode;
	bool m_bLoginConnected;
	std::string m_xLoginAddr;
	DWORD m_dwLsConnIndex;
	bool m_bAppException;

	//	服务器的监听端口
	WORD m_dwListenPort;
	std::string m_xListenIP;

	// Distinct ip set
	std::map<std::string, int> m_xIPs;

	//	NetThreadEvent processed by timer loop
	NetThreadEventList m_xNetThreadEventList;
	std::mutex m_csNetThreadEventList;

	ServerShell *m_pServerShell;

public:
	HWND m_hDlgHwnd;
};

#endif
