/*#ifndef _INC_WORLDEVENTDISPATCHER_
#define _INC_WORLDEVENTDISPATCHER_
//////////////////////////////////////////////////////////////////////////
#include "../../CommonModule/LuaEventDispatcher.h"
//////////////////////////////////////////////////////////////////////////
enum WorldEvent
{
	kWorldEvent_None,
	//	������� 1s����һ��
	kWorldEvent_Update,
	//	��ʱ���񴥷�
	kWorldEvent_ScheduleActive,
	//	NPC���
	kWorldEvent_NPCActive,
	//	��������
	kWorldEvent_StartRunning,
};

struct WorldEvent_ScheduleActive
{
	int nScheduleId;

	WorldEvent_ScheduleActive()
	{
		nScheduleId = 0;
	}
};

class NPCObject;
class HeroObject;

struct WorldEvent_NPCActive
{
	int nButtonId;
	NPCObject* pNPC;
	HeroObject* pHero;

	WorldEvent_NPCActive()
	{
		nButtonId = 0;
		pNPC = NULL;
		pHero = NULL;
	}
};
//////////////////////////////////////////////////////////////////////////
class WorldEventDispatcher : public LuaEventDispatcher
{
public:
	WorldEventDispatcher();
	virtual ~WorldEventDispatcher();

public:
	virtual int OnDispatchEvent(const LuaDispatchEvent* _pEvent, LuaDispatchInfo* _pInfo);
	virtual void OnDispatchEventResult(const LuaDispatchEvent* _pEvent, bool bResult);
};
//////////////////////////////////////////////////////////////////////////
#endif*/