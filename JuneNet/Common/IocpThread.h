/*
FileName: IocpThread.h
Description: IocpThread
Autor: LeeYouHong
*/

#pragma once

#ifndef __IOCPTHREAD_H__
#define __IOCPTHREAD_H__

#include <thread>
#include <WinSock2.h>

#define MAX_IOCP_THREAD 50

namespace JuneNet
{

	class CIocpThread
	{
	public:
		CIocpThread();
		~CIocpThread();

	public:
		static int IocpThreadProc(void *pParam);

		bool Create(int threadNum = 0);

		void Destroy();

		HANDLE GetIocpHandle();

	public:
		HANDLE m_hIOCP;
		int    m_ThreadNum;
		std::thread m_Thread[MAX_IOCP_THREAD];

	};
}

#endif