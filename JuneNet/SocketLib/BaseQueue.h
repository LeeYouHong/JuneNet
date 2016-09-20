/*
FileName: BaseQueue.h
Description: 接收、发送数据队列的基类
Autor: LeeYouHong
*/

#pragma once

#ifndef __BASEQUEUE_H__
#define __BASEQUEUE_H__

#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE	4096
#endif

#ifndef MIN_BLOCK_SIZE
#define MIN_BLOCK_SIZE		32
#endif

#ifndef MAX_QUEUE_SIZE
#define MAX_QUEUE_SIZE		2097152
#endif

#include <array>
#include <queue>

namespace JuneNet
{
	class CBuffer
	{
	public:
		int m_nStartPos = 0;
		int m_nEndPos = 0;
		std::array<char, MAX_QUEUE_SIZE> m_Buffer;

		int GetDataSize(){ return m_nEndPos - m_nStartPos; }

		char* GetData(){ return m_Buffer.data() + m_nStartPos; }
		
		char* GetFreeData(){ return m_Buffer.data() + m_nEndPos; }

		int GetFreeSize(){ return m_Buffer.max_size() - m_nEndPos; }
	};

	using BufferQueueType = std::queue<CBuffer*>;

	class CBaseQueue
	{
	public:
		CBaseQueue()
			:m_nNowSize(0)
		{
		};

		~CBaseQueue()
		{
		};

	protected:
		BufferQueueType m_BufferQueue;	// 数据队列，先进先出
		int m_nNowSize;

	public:
		int GetSize(){ return m_nNowSize; }
	};
}

#endif