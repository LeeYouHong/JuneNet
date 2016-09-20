#include "SocketManager.h"

using namespace JuneNet;

JuneNet::CSocketManager::CSocketManager()
	: m_hWs2Dll(nullptr)
	, m_ServerSocket(nullptr)
	, m_nLastAcceptIdx(1)
{
	for (int i = 0; i < MAX_DATA_SOCKET; ++i)
	{
		m_SocketList[i] = nullptr;
	}

	m_ServerSocket = nullptr;
	m_nLastAcceptIdx = 1;
}

JuneNet::CSocketManager::~CSocketManager()
{
	Destroy();
}

bool JuneNet::CSocketManager::Init(HWND hWnd,int threadNum /*= 0*/)
{
	Destroy();

	m_hWs2Dll = LoadLibraryA("WS2_32.DLL");

	if (m_hWs2Dll == nullptr)
	{
		return false;
	}


	WSADATA wd = { 0 };
	int errCode = WSAStartup(MAKEWORD(2, 2), &wd);
	SetLastError(errCode);

	if (errCode != 0)
	{
		return false;
	}

	if (LOBYTE(wd.wVersion) < 2)
	{
		return false;
	}

	if (!m_IocpThread.Create(threadNum))
	{
		return false;
	}

	m_hIOCP = m_IocpThread.GetIocpHandle();
	m_hWindows = hWnd;

	for (int i = 0; i < MAX_DATA_SOCKET; ++i)
	{
		if (m_SocketList[i] == nullptr)
		{
			m_SocketList[i] = new CMySocket(m_hWindows, m_hIOCP);
		}
	}

	return true;
}

void JuneNet::CSocketManager::Destroy()
{
	m_IocpThread.Destroy();

	CloseServerSocket();

	for (int i = 0; i < MAX_DATA_SOCKET; ++i)
	{
		if (m_SocketList[i] != nullptr)
		{
			m_SocketList[i]->Close();
			m_SocketList[i]->Destroy();
			delete m_SocketList[i];
			m_SocketList[i] = nullptr;
		}
	}

	if (m_hWs2Dll != nullptr)
	{
		WSACleanup();
		FreeLibrary(m_hWs2Dll);
		m_hWs2Dll = nullptr;
	}


}

bool JuneNet::CSocketManager::CreateServerSocket(int port, std::string& ip)
{
	CloseServerSocket();

	m_ServerSocket = new CMySocket(m_hWindows, m_hIOCP);

	if (m_ServerSocket == nullptr)
	{
		return false;
	}

	if (!m_ServerSocket->Create())
	{
		return false;
	}

	if (!m_ServerSocket->Bind(port, ip))
	{
		return false;
	}

	if (!m_ServerSocket->Listen(500))
	{
		return false;
	}

	return true;
}

bool JuneNet::CSocketManager::CloseServerSocket()
{
	if (m_ServerSocket != nullptr)
	{
		m_ServerSocket->Destroy();
		delete m_ServerSocket;
		m_ServerSocket = nullptr;
	}

	return true;
}

CMySocket* JuneNet::CSocketManager::GetAcceptibleSocket(int& nIdx)
{
	for (int i = 0; i < MAX_DATA_SOCKET; ++i)
	{
		CMySocket *pSocket = m_SocketList[i];
		if (pSocket == nullptr) continue;
		if (pSocket->m_bCreated) continue;

		nIdx = i;
		return pSocket;
	}

	return nullptr;
}

int JuneNet::CSocketManager::AcceptSocket()
{
	if (m_ServerSocket == nullptr)
	{
		return -1;
	}

	int nIdx = -1;
	CMySocket *pSocket = GetAcceptibleSocket(nIdx);
	if (pSocket != nullptr)
	{
		pSocket->SetSocketID(nIdx);

		if (!m_ServerSocket->Accept(pSocket))
		{
			pSocket->Destroy();
			return -1;
		}

		AddKeepSocket(pSocket);

		return nIdx;
	}

	SOCKET s = accept(m_ServerSocket->m_hSocket, nullptr, nullptr);
	if (s != INVALID_SOCKET)
	{
		closesocket(s);
	}

	return -1;
}

bool JuneNet::CSocketManager::AddKeepSocket(CMySocket *pSocket, bool bConnect)
{
	for (auto tmp : m_KeepList)
	{
		if (tmp == nullptr)
		{
			continue;
		}

		if (tmp == pSocket)
		{
			return false;
		}
	}

	m_KeepList.push_back(pSocket);

	return true;
}

bool JuneNet::CSocketManager::DelKeepSocket(CMySocket *pSocket)
{
	for (auto iter = m_KeepList.begin(); iter != m_KeepList.end();)
	{
		CMySocket *tmp = *iter;
		if (tmp == nullptr)
		{
			iter = m_KeepList.erase(iter);
			continue;
		}

		if (tmp == pSocket)
		{
			iter = m_KeepList.erase(iter);
			return true;
		}

		++iter;
	}

	return false;
}

bool JuneNet::CSocketManager::RearrangeKeepSocket(CMySocket *pSocket)
{
	for (auto iter = m_KeepList.begin(); iter != m_KeepList.end();)
	{
		CMySocket *tmp = *iter;
		if (tmp == nullptr)
		{
			++iter;
			continue;
		}

		if (tmp == pSocket)
		{
			iter = m_KeepList.erase(iter);
			m_KeepList.push_back(pSocket);

			return true;
		}

		++iter;
	}

	return false;
}

bool JuneNet::CSocketManager::OnPacketRecv(CMySocket *pSocket, int nPacketCnt, int nSize)
{
	return true;
}

bool JuneNet::CSocketManager::ReservClose(int sid)
{
	if (sid < 0 || sid >= MAX_DATA_SOCKET) return false;
	if (m_SocketList[sid] == nullptr) return false;

	m_SocketList[sid]->ReserveClose();

	return true;
}

bool JuneNet::CSocketManager::CloseDataSocket(int sid)
{
	if (sid < 0 || sid >= MAX_DATA_SOCKET) return false;
	if (m_SocketList[sid] == nullptr) return false;
	if (!m_SocketList[sid]->m_bCreated) return false;

	DelKeepSocket(m_SocketList[sid]);

	return m_SocketList[sid]->Close();
}

bool JuneNet::CSocketManager::CloseDataSocket(CMySocket *pSocket)
{
	if (pSocket == nullptr) return false;
	if (!pSocket->m_bCreated) return false;

	return CloseDataSocket(pSocket->m_UserVal);
}

CMySocket* JuneNet::CSocketManager::GetDataSocketPtr(int sid)
{
	if (sid < 0 || sid >= MAX_DATA_SOCKET) return nullptr;
	return m_SocketList[sid];
}
