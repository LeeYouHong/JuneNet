#include "SendQueue.h"

using namespace JuneNet;

CMemoryPool<CBuffer> CSendQueue::s_BufferPool;

CSendQueue::CSendQueue()
{

}

CSendQueue::~CSendQueue()
{

}

void CSendQueue::Destroy()
{
	CBuffer *pBuffer = nullptr;

	while (!m_BufferQueue.empty())
	{
		pBuffer = m_BufferQueue.front();
		if (pBuffer != nullptr)
		{
			ReleaseBufferQueue();
		}
	}

	m_nNowSize = 0;
}

char* CSendQueue::GetBufPtr()
{
	if (m_BufferQueue.empty()) return nullptr;

	CBuffer *pBuffer = m_BufferQueue.front();
	if (pBuffer == nullptr) return nullptr;

	return pBuffer->GetData();
}

int CSendQueue::GetBlockSize()
{
	if (m_BufferQueue.empty()) return 0;

	CBuffer *pBuffer = m_BufferQueue.front();
	if (pBuffer == nullptr) return 0;

	return pBuffer->GetDataSize();
}

bool CSendQueue::EnQueue(void *pData, int size)
{
	if (pData == nullptr || size <= 0) return false;

	if (m_nNowSize + size > MAX_QUEUE_SIZE) return false;

	CBuffer *pBuffer = nullptr;
	if (m_BufferQueue.empty())
	{
		pBuffer = AddNewBufferQueue();
	}
	else
	{
		pBuffer = m_BufferQueue.back();
	}

	int remainSize = size;

	while (remainSize > 0)
	{
		int freeSize = pBuffer->GetFreeSize();
		if (remainSize <= freeSize)
		{
			memcpy(pBuffer->GetData(), (char*)pData + (size - remainSize), remainSize);
			pBuffer->m_nEndPos += remainSize;
			remainSize = 0;
			break;
		}
		else
		{
			if (pBuffer->m_Buffer.max_size() - pBuffer->m_nStartPos >= MIN_BLOCK_SIZE)
			{
				memcpy(pBuffer->GetData(), (char*)pData + (size - remainSize), freeSize);
				pBuffer->m_nEndPos += freeSize;
				remainSize -= freeSize;
			}

			pBuffer = AddNewBufferQueue();
			if (pBuffer == nullptr) return false;
		}
	}

	m_nNowSize += size;

	return true;
}

bool CSendQueue::DeQueueSize(int size)
{
	if (m_nNowSize <= 0 || m_BufferQueue.empty()) return false;

	int remainSize = (size > m_nNowSize) ? m_nNowSize : size;

	while (remainSize > 0)
	{
		if (m_BufferQueue.empty()) break;

		CBuffer *pBuffer = m_BufferQueue.front();
		int dataSize = pBuffer->GetDataSize();
		if (dataSize > remainSize)
		{
			pBuffer->m_nStartPos += remainSize;
			m_nNowSize -= remainSize;
			remainSize = 0;
		}
		else
		{
			remainSize -= dataSize;
			m_nNowSize -= dataSize;
			pBuffer->m_nStartPos = 0;
			pBuffer->m_nEndPos = 0;

			if (m_BufferQueue.size() > 1)
			{
				ReleaseBufferQueue();
			}
		}
	}

	if (m_nNowSize < 0) m_nNowSize = 0;

	return true;
}

CBuffer* CSendQueue::AddNewBufferQueue()
{
	CBuffer* pBuffer = s_BufferPool.Get();

	if (pBuffer == nullptr) return nullptr;

	pBuffer->m_nStartPos = 0;
	pBuffer->m_nEndPos = 0;

	m_BufferQueue.push(pBuffer);

	return pBuffer;
}

void CSendQueue::ReleaseBufferQueue()
{
	if (!m_BufferQueue.empty())
	{
		CBuffer *pBuffer = m_BufferQueue.front();
		m_BufferQueue.pop();
		s_BufferPool.Release(pBuffer);
	}
}

int CSendQueue::GetBufferPoolSize()
{
	return s_BufferPool.GetPoolSize();
}
