/*
FileName: SendQueue.h
Description: �������ݶ��У�Ӧ�ò�����ݷŵ��ö����У�����ÿ���з������������ڴ�
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