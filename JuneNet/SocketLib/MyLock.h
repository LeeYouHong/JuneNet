/*
FileName: MyLock.h
Description: 临界锁，保护某段代码区同一时刻只有一个线程执行
Autor: LeeYouHong
*/

#pragma once

#ifndef __MYLOCK_H__
#define __MYLOCK_H__

#include <windows.h>

namespace JuneNet
{
	class CLockGuard
	{
	public:
		CLockGuard(CRITICAL_SECTION *cs)
		{
			m_pCS = cs;
			EnterCriticalSection(m_pCS);
		}

		~CLockGuard()
		{
			LeaveCriticalSection(m_pCS);
		}

	private:
		CRITICAL_SECTION *m_pCS;
	};

	class CMyLock
	{
	public:
		CRITICAL_SECTION m_cs;

		CMyLock()
		{
			InitializeCriticalSection(&m_cs);
		};

		~CMyLock()
		{
			DeleteCriticalSection(&m_cs);
		};

		inline void Lock()
		{
			EnterCriticalSection(&m_cs);
		};

		inline void Unlock()
		{
			LeaveCriticalSection(&m_cs);
		};
	};
}

#endif