#ifndef _INC_MONSTERTEMPLATEOBJECT_
#define _INC_MONSTERTEMPLATEOBJECT_
//////////////////////////////////////////////////////////////////////////
#include "MonsterObject.h"
#include <vector>
//////////////////////////////////////////////////////////////////////////
enum ATTACK_MODE
{
	ATTACK_NONE,
	ATTACK_NORMAL,
	ATTACK_SPECIAL
};

struct MonsLogicItem
{
public:
	MonsLogicItem()
	{
		m_nLogicID = 0;
		m_nActionCode = ACTION_NONE;
		m_dwLastExecuteTime = 0;
		m_dwRecoverTime = 800;
		m_nProbability = 0;
		m_bExecutable = true;
		m_dwExecuteInterval = 0;
	}

public:
	//	id
	int m_nLogicID;
	//	will send to client,determine which special action
	int m_nActionCode;
	//	last execute time
	DWORD m_dwLastExecuteTime;
	//	recover time , to stand state
	DWORD m_dwRecoverTime;
	//	execute probability
	int m_nProbability;
	//	executable
	bool m_bExecutable;
	//	execute interval
	DWORD m_dwExecuteInterval;
};
typedef std::vector<MonsLogicItem> MonsLogicList;

class MonsterTemplateObject : public MonsterObject
{
public:
	MonsterTemplateObject();
	virtual ~MonsterTemplateObject();

public:
	virtual bool CanAttack();
	virtual void DoAction(unsigned int _dwTick);
	virtual void AttackHero();
	virtual bool AttackTarget();

	//	add
	virtual bool LogicCanAttack(const MonsLogicItem* _pLogic){return false;}
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic){return false;};

	//	stand״̬ʱ�� ������һЩ����
	virtual bool FreeUpdate()	{return true;}

public:
	MonsLogicItem* GetCurrentLogic();
	void AddLogicItem(int _nID, int _nActionCode, int _nProb, int _nExecuteInterval, int _nRecoverTime = 0);
	MonsLogicItem* GetLogicByID(int _nID);

	void ResetLogicAttackStatus()
	{
		m_nExecuteLogicIndex = -1;
		m_eAttackMode = ATTACK_NONE;
	}

	int GetMoveOffsetXByDirection(int _nDrt);
	int GetMoveOffsetYByDirection(int _nDrt);
	int GetDirectionByMoveOffset(int _nOftX, int _nOftY);

protected:
	virtual void SelectAttackMode();

protected:
	MonsLogicList m_xMonsLogicList;
	int m_nExecuteLogicIndex;
	DWORD m_dwLastRecoverTime;
	ATTACK_MODE m_eAttackMode;
};

//////////////////////////////////////////////////////////////////////////
class MoonLizardMonster : public MonsterTemplateObject
{
public:
	MoonLizardMonster();

public:
	//	add
	virtual bool LogicCanAttack(const MonsLogicItem* _pLogic);
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic);
};

//////////////////////////////////////////////////////////////////////////
class MoonStatueMonster : public MonsterTemplateObject
{
public:
	MoonStatueMonster();

public:
	//	add
	virtual bool LogicCanAttack(const MonsLogicItem* _pLogic);
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic);
};
//////////////////////////////////////////////////////////////////////////

class MoonSpiderMonster : public MonsterTemplateObject
{
public:
	MoonSpiderMonster();

public:
	//	add
	virtual bool LogicCanAttack(const MonsLogicItem* _pLogic);
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic);
};

//////////////////////////////////////////////////////////////////////////
class MoonBeastMonster : public MonsterTemplateObject
{
public:
	MoonBeastMonster();

public:
	//	add
	virtual bool LogicCanAttack(const MonsLogicItem* _pLogic);
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic);
}; 
//////////////////////////////////////////////////////////////////////////
class MoonWarLordMonster : public MonsterTemplateObject
{
public:
	MoonWarLordMonster();

public:
	virtual DWORD GetAttackCostTime();
	virtual int GetRandomAbility(ABILITY_TYPE _type);
	virtual void DoAction(unsigned int _dwTick);
	virtual void AttackHero();
	//	add
	virtual bool LogicCanAttack(const MonsLogicItem* _pLogic);
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic);

protected:
	virtual void SelectAttackMode();

protected:
	DWORD m_dwLastDefenceTime;
	DWORD m_dwLastFlyTime;
	DWORD m_dwLastSummonTime;
	DWORD m_dwLastAttackHeroTime;
}; 
//////////////////////////////////////////////////////////////////////////
class YamaKingMonster : public MonsterTemplateObject
{
public:
	YamaKingMonster();

public:
	virtual bool CanAttack();
	virtual bool AttackTarget();

	virtual bool LogicCanAttack(const MonsLogicItem* _pLogic);
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic);

public:
	bool NormalAttack();
	bool ExtAttack0();
	bool ExtAttack1();
	bool ExtAttack2();
	bool ExtAttack3();

	void SummonSlaves();
	void UpdateLogicProb();

protected:
	DWORD m_dwLastSummonTime;
};
//////////////////////////////////////////////////////////////////////////
class FireDragonMonster : public MonsterTemplateObject
{
public:
	FireDragonMonster();

public:
	virtual bool CanAttack();
	virtual bool AttackTarget();
	//virtual void ParseAttackMsg(AttackMsg* _pMsg);
	virtual void OnDeadRevive(int _nLeftLife);
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic);
	virtual int GetRandomAbility(ABILITY_TYPE _type);
	virtual void OnMonsterDead(HeroObject* _pAttacher, bool _bKilledBySlave);

public:
	bool NormalAttack();
	bool ExtAttack0();
	int m_nDeadTimes;
};
//////////////////////////////////////////////////////////////////////////
class SuperBullKing : public MonsterTemplateObject
{
public:
	SuperBullKing();

public:
	virtual bool LogicCanAttack(const MonsLogicItem* _pLogic);
	virtual bool LogicAttackTarget(const MonsLogicItem* _pLogic);
	virtual bool FreeUpdate();
	virtual int GetRandomAbility(ABILITY_TYPE _type);
	virtual int ReceiveDamage(GameObject* _pAttacker, bool _bMgcAtk, int _oriDC = 0, int _nDelay = 350, const ReceiveDamageInfo* _pInfo = NULL);

public:
	bool ExtAttack0();

private:
	DWORD m_dwLastFlyTime;
};
//////////////////////////////////////////////////////////////////////////
class DevilOldManMonster : public MonsterTemplateObject
{
public:
	DevilOldManMonster();
	
public:
	virtual bool AttackTarget();
	//virtual int ReceiveDamage(GameObject* _pAttacker, bool _bMgcAtk, int _oriDC /* = 0 */, int _nDelay /* = 350 */, const ReceiveDamageInfo* _pInfo /* = NULL */);
	virtual void ParseAttackMsg(AttackMsg* _pMsg);
};
//////////////////////////////////////////////////////////////////////////
class QieEMonster : public MonsterTemplateObject
{
public:
	QieEMonster();

public:
	virtual bool AttackTarget();
};
//////////////////////////////////////////////////////////////////////////
#endif