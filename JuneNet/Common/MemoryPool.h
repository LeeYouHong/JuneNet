/*
FileName: MemoryPool.h
Description: Memory Pool
Autor: LeeYouHong
*/

#pragma once

#ifndef __MEMORYPOOL_H__
#define __MEMORYPOOL_H__

#include <windows.h>
#include <stack>

namespace JuneNet
{
	template <class T>
	class CMemoryPool
	{
	public:
		CMemoryPool()
		{
			InitializeCriticalSectionAndSpinCount(&m_CS, 4000);
		}

		virtual ~CMemoryPool()
		{
			while (!m_FreeList.empty())
			{
				T* pObj = m_FreeList.top();
				if (pObj)
				{
					delete pObj;
				}
				m_FreeList.pop();
			}

			DeleteCriticalSection(&m_CS);
		}

	public:
		T* Get()
		{
			T *pObj = nullptr;

			EnterCriticalSection(&m_CS);

			if (m_FreeList.empty())
			{
				pObj = new T;
			}
			else
			{
				pObj = (T*)m_FreeList.top();
				m_FreeList.pop();
			}

			LeaveCriticalSection(&m_CS);

			return pObj;
		}

		void Release(T* pObj)
		{
			EnterCriticalSection(&m_CS);
			m_FreeList.push(pObj);
			LeaveCriticalSection(&m_CS);
		}

		int GetPoolSize()
		{
			return m_FreeList.size();
		}

	protected:
		std::stack<T*> m_FreeList;
		CRITICAL_SECTION m_CS;
	};
}

#endif