// BackMirServer.h : BackMirServer Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CBackMirServerApp:
// �йش����ʵ�֣������ BackMirServer.cpp
//

class CBackMirServerApp : public CWinApp
{
public:
	CBackMirServerApp();


// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CBackMirServerApp theApp;