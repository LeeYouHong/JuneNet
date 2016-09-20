/*
FileName: RecvQueue.h
Description: Recv Queue
Autor: LeeYouHong
*/

#pragma once

#ifndef __RECVQUEUE_H__
#define __RECVQUEUE_H__

#include "BaseQueue.h"
#include "MemoryPool.h"

namespace JuneNet
{
	class CRecvQueue : public CBaseQueue
	{
	public:
		CRecvQueue();
		~CRecvQueue();

	public:
		void Destroy();

		int GetFreeBufPtr(char* &ptr);

		int GetBufPtr(char* &ptr);

	public:
		bool EnQueueSize(int size);
		bool PeekQueue(void *pData, int size);
		bool DeQueue(void *pData, int size);
		bool DeQueueSize(int size);

	protected:
		void ReleaseBufferQueue();

		CBuffer* AddNewBufferQueue();

		int GetBufferPoolSize();

	private:
		static CMemoryPool<CBuffer> s_BufferPool;
	};
}

#endif