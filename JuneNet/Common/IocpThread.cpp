#include "IocpThread.h"

#include "SocketItem.h"

using namespace JuneNet;

CIocpThread::CIocpThread()
	: m_hIOCP(nullptr)
	, m_ThreadNum(0)
{
	memset(m_Thread, 0, sizeof(m_Thread));
}

CIocpThread::~CIocpThread()
{
	Destroy();
}

int CIocpThread::IocpThreadProc(void *pParam)
{
	CIocpThread *pInstance = (CIocpThread*) pParam;
	HANDLE hIOCP = pInstance->m_hIOCP;

	int transByte = 0;
	MYOVERLAPPED *pMyOVl = nullptr;
	CSocketItem* pSocket = nullptr;

	while (true)
	{
		if (GetQueuedCompletionStatus(hIOCP, reinterpret_cast<DWORD*>(&transByte), reinterpret_cast<PULONG_PTR>(&pSocket),
			reinterpret_cast<LPOVERLAPPED*>(&pMyOVl), INFINITE) == 0)
		{
			if (GetLastError() == ERROR_NETNAME_DELETED)
			{
				pSocket->DisconnFeedBack();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			continue;
		}

		if (transByte > 0)
		{
			switch (pMyOVl->OperationType)
			{
			case OPERATION_SEND:
				pSocket->SendFeedBack(transByte);
				break;

			case OPERATION_RECV:
				pSocket->ReceiveFeedBack(transByte);
				break;
			}
		}
		else if (transByte == 0)
		{
			pSocket->DisconnFeedBack();
		}
		else if (transByte == IOCP_SOCKETCLOSE)
		{
			if (pSocket == nullptr)
			{
				pSocket->CloseFeedBack();
			}
		}
		else if (transByte == IOCP_THREADCLOSE)
		{
			return 0;
		}
	}
}

bool CIocpThread::Create(int threadNum /*= 0*/)
{
	if (m_hIOCP != nullptr)
	{
		Destroy();
	}

	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (m_hIOCP == nullptr)
	{
		return false;
	}

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	int thdNum = max(threadNum, min(systemInfo.dwNumberOfProcessors * 2, MAX_IOCP_THREAD));
	for (int i = 0; i < thdNum; ++i)
	{
		m_Thread[i] = std::thread(IocpThreadProc, this);
		m_Thread[i].detach();
	}

	m_ThreadNum = thdNum;

	return true;
}

void CIocpThread::Destroy()
{
	for (int i = 0; i < m_ThreadNum; ++i)
	{
		PostQueuedCompletionStatus(m_hIOCP, IOCP_THREADCLOSE, 0, nullptr);
	}
}

HANDLE CIocpThread::GetIocpHandle()
{
	return m_hIOCP;
}
