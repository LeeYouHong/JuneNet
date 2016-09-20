/*
FileName: IocpThread.h
Description: 完成端口的工作线程逻辑，处理套接字的读、写完成事件
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