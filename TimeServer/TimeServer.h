
// TimerServer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTimerServerApp: 
// �йش����ʵ�֣������ TimerServer.cpp
//

class CTimerServerApp : public CWinApp
{
public:
	CTimerServerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTimerServerApp theApp;