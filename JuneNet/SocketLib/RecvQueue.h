/*
FileName: RecvQueue.h
Description: 接收数据队列，把收到的数据放进该队列，可适应任何大小的消息包
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