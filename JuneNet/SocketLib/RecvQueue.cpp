#include "RecvQueue.h"

using namespace JuneNet;

CMemoryPool<CBuffer> CRecvQueue::s_BufferPool;

CRecvQueue::CRecvQueue()
{
	
}


CRecvQueue::~CRecvQueue()
{

}

void CRecvQueue::Destroy()
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

int CRecvQueue::GetFreeBufPtr(char* &ptr)
{
	ptr = nullptr;
	if (m_BufferQueue.empty())
	{
		AddNewBufferQueue();
	}

	CBuffer *pBuffer = m_BufferQueue.back();
	if (pBuffer == nullptr) return 0;

	int freeSize = pBuffer->m_Buffer.max_size() - pBuffer->m_nEndPos;
	if (freeSize < MIN_BLOCK_SIZE) // ¿Õ¼ä²»¹»
	{
		AddNewBufferQueue();
	}

	pBuffer = m_BufferQueue.back();
	if (pBuffer == nullptr) return 0;

	ptr = pBuffer->GetData();

	return pBuffer->GetFreeSize();
}

int CRecvQueue::GetBufPtr(char* &ptr)
{
	ptr = nullptr;

	if (m_BufferQueue.empty()) return 0;

	CBuffer *pBuffer = m_BufferQueue.front();
	if (pBuffer == nullptr) return 0;

	if (pBuffer->GetDataSize() == 0 && m_BufferQueue.size() > 1)
	{
		ReleaseBufferQueue();
	}

	pBuffer = m_BufferQueue.front();
	if (pBuffer == nullptr) return 0;

	ptr = pBuffer->GetData();

	return pBuffer->GetDataSize();
}

bool CRecvQueue::EnQueueSize(int size)
{
	if (size <= 0 || m_BufferQueue.empty())
	{
		return false;
	}

	if (m_nNowSize + size > MAX_QUEUE_SIZE)
	{
		return false;
	}

	CBuffer *pBuf = m_BufferQueue.back();
	if (!pBuf)
	{
		return false;
	}

	if (pBuf->m_nEndPos + size > pBuf->m_Buffer.max_size())
	{
		return false;
	}

	pBuf->m_nEndPos += size;
	m_nNowSize += size;

	return true;
}

bool CRecvQueue::PeekQueue(void *pData, int size)
{
	if (pData == nullptr || size <= 0) return false;
	if (m_BufferQueue.empty()) return false;
	if (size > m_nNowSize) return false;

	int remainSize = size;

	auto Container = m_BufferQueue._Get_container();
	for (auto iter = Container.begin(); iter != Container.end(); iter++)
	{
		if (*iter == nullptr) break;

		int dataSize = (*iter)->GetDataSize();
		
		if ( remainSize < dataSize)
		{
			memcpy((char*)pData + (size - remainSize), (*iter)->GetData(), remainSize);
			break;
		}
		else
		{
			memcpy((char*)pData + (size - remainSize), (*iter)->GetData(), dataSize);
		}

		remainSize -= dataSize;
	}

	return true;
}

bool CRecvQueue::DeQueue(void *pData, int size)
{
	if (!PeekQueue(pData, size))
		return false;

	return DeQueueSize(size);
}

bool CRecvQueue::DeQueueSize(int size)
{
	if (m_nNowSize <= 0 || m_BufferQueue.empty()) return false;

	int tmpSize = size > m_nNowSize ? m_nNowSize : size;

	while (tmpSize > 0 && !m_BufferQueue.empty())
	{
		CBuffer *pBuf = m_BufferQueue.front();

		int dataSize = pBuf->GetDataSize();

		if (tmpSize < dataSize)
		{
			pBuf->m_nStartPos += tmpSize;
			m_nNowSize -= tmpSize;
			break;
		}
		else
		{
			tmpSize -= dataSize;
			m_nNowSize -= dataSize;

			pBuf->m_nStartPos = pBuf->m_nEndPos;

			if (pBuf->m_nStartPos == pBuf->m_Buffer.max_size())
			{
				pBuf->m_nStartPos = 0;
				pBuf->m_nEndPos = 0;
			}

			if (m_BufferQueue.size() > 1)
			{
				ReleaseBufferQueue();
			}

		}
	}

	if (m_nNowSize < 0) m_nNowSize = 0;

	return true;
}

void CRecvQueue::ReleaseBufferQueue()
{
	if (!m_BufferQueue.empty())
	{
		CBuffer *pBuffer = m_BufferQueue.front();
		m_BufferQueue.pop();
		s_BufferPool.Release(pBuffer);
	}
}

CBuffer* CRecvQueue::AddNewBufferQueue()
{
	CBuffer *pBuffer = s_BufferPool.Get();

	if (pBuffer == nullptr) return nullptr;

	pBuffer->m_nStartPos = 0;
	pBuffer->m_nEndPos = 0;

	m_BufferQueue.push(pBuffer);

	return pBuffer;
}

int CRecvQueue::GetBufferPoolSize()
{
	return s_BufferPool.GetPoolSize();
}
