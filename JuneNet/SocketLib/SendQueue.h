/*
FileName: SendQueue.h
Description: 发送数据队列，应用层的数据放到该队列中，不用每次有发送请求都申请内存
Autor: LeeYouHong
*/

#pragma once

#ifndef __SENDQUEUE_H__
#define __SENDQUEUE_H__

#include "BaseQueue.h"
#include "MemoryPool.h"

namespace JuneNet
{

	class CSendQueue : public CBaseQueue
	{
	public:
		CSendQueue();
		~CSendQueue();

	public:
		void Destroy();

		char* GetBufPtr();

		int GetBlockSize();

		bool EnQueue(void *pData, int size);
		bool DeQueueSize(int size);

	protected:
		CBuffer* AddNewBufferQueue();

		void ReleaseBufferQueue();

		int GetBufferPoolSize();

	private:
		static CMemoryPool<CBuffer> s_BufferPool;

	};
}

#endif