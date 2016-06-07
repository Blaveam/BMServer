#ifndef PACKET_H_
#define PACKET_H_

#include <Windows.h>
#include <list>

enum PACKET_COMMAND
{
	PC_BEGIN = 0,

	//	ϵͳ���ݰ�
	PC_SYSTEM_LOGIN_REQ = 1,
	PC_SYSTEM_LOGIN_ACK,

	//	���ݰ����ʼ

	//	���ﶯ��
	PC_ACTION_BEGIN = 0x00FF,
	//	�߶�
	PC_ACTION_WALK,
	//	�ܶ�
	PC_ACTION_RUN,
	//	ת��
	PC_ACTION_TURN,
	//	����
	PC_ACTION_ATTACK,

	PC_ACTION_END,

	//	�������
	PC_END = 0xFFFF,
};

struct PacketBase
{
	//	����
	DWORD dwSize;
	//	����
	WORD wCmd;
	//	����ID
	DWORD dwObjID;
	//	Ŀ��ID
	DWORD dwTarget;
};

struct ActionPacket : public PacketBase
{
	//	����
	WORD wParam0;
	WORD wParam1;
	WORD wParam2;
	WORD wParam3;
	WORD wParam4;
};

struct UserLoginPacket : public PacketBase
{
	//
	char szName[20];
};

struct UserLoginPacketAck : public PacketBase
{
	//
	BYTE bRet;
};

typedef std::list<ActionPacket*> ACTIONLIST;

// class PacketReader
// {
// public:
// 	BOOL Read(const char* _pData, DWORD _dwLen,)
// };


#endif