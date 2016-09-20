#include "SocketItem.h"
//#include <windows.h>
#include <tchar.h>

#pragma comment(lib, "ws2_32.lib")

using namespace JuneNet;

static CMyLock AcceptLock;
static CMyLock ConnectLock;
static CMyLock CloseLock;

JuneNet::CSocketItem::CSocketItem(HANDLE hIocp)
	:m_hIocp(hIocp)
{
	Clear();

	InitializeCriticalSectionAndSpinCount(&m_CS, 4000);
}

JuneNet::CSocketItem::~CSocketItem()
{
	Destroy();
	DeleteCriticalSection(&m_CS);
}

bool JuneNet::CSocketItem::Destroy()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

	m_bCreated = false;
	m_bIsOnline = false;
	m_bIsListeningSock = false;
	m_bUseForSend = false;
	m_nCloseTime = 0;

	memset(&m_SendOvl, 0, sizeof(MYOVERLAPPED));
	memset(&m_RecvOvl, 0, sizeof(MYOVERLAPPED));

	m_SendQueue.Destroy();
	m_RecvQueue.Destroy();

	m_bFlushMode = false;
	m_bNowFlushing = false;

	return true;
}

void JuneNet::CSocketItem::Clear()
{
	m_hSocket = INVALID_SOCKET;
	m_bCreated = false;
	m_bIsOnline = false;
	m_bIsListeningSock = false;
	m_nCloseTime = 0;
	m_bUseForSend = false;
	m_bClosePosted = false;

	m_bFlushMode = false;
	m_bNowFlushing = false;
	
	memset(&m_SendOvl, 0, sizeof(MYOVERLAPPED));
	memset(&m_RecvOvl, 0, sizeof(MYOVERLAPPED));

	m_UserVal = 0;

	m_hMsgWnd = nullptr;
}

bool JuneNet::CSocketItem::Create()
{
	if (m_bCreated || m_hSocket != INVALID_SOCKET)
	{
		m_LastError << "Socket is already created.";
		return false;
	}

	m_hSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (m_hSocket == INVALID_SOCKET)
	{
		m_LastError << "Creating socket failed.";
		m_hSocket = INVALID_SOCKET;
		return false;
	}

	m_bCreated = true;

	return true;
}

bool JuneNet::CSocketItem::Close()
{
	Lock();
	if (m_hSocket != INVALID_SOCKET)
	{
		shutdown(m_hSocket, SD_BOTH);
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

	Destroy();

	UnLock();

	return true;
}

bool JuneNet::CSocketItem::ReserveClose()
{
	if (!m_bCreated || !m_bIsOnline || m_bClosePosted || m_hSocket == INVALID_SOCKET) return false;

	Lock();

	PostClose();

	UnLock();

	return true;
}

bool JuneNet::CSocketItem::Bind(int nServerPort, const std::string& ip)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		m_LastError << "Socket not initialized.";
		return false;
	}

	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = ip.empty() ? htonl(INADDR_ANY) : inet_addr(ip.data());
	sockAddr.sin_port = htons(static_cast<u_short>(nServerPort));

	if (bind(m_hSocket, reinterpret_cast<sockaddr*>(&sockAddr), sizeof(sockAddr)) == SOCKET_ERROR)
	{
		m_LastError << "bind() error : " << WSAGetLastError();
		return false;
	}

	return true;
}

bool JuneNet::CSocketItem::Listen(int nBackLog /*= 5*/)
{
	if (m_bIsListeningSock)
	{
		m_LastError << "Socket is already listening.";
		return false;
	}

	if (m_hSocket == INVALID_SOCKET)
	{
		m_LastError << "Socket is invalid.";
		return false;
	}

	if (CreateMsgWindow(nullptr) == false)
	{
		m_LastError << "CreateMsgWindow failed.";
		return false;
	}

	if (WSAAsyncSelect(m_hSocket, m_hMsgWnd, UM_FASTSOCKET_NOTIFY, FD_ACCEPT) == SOCKET_ERROR)
	{
		m_LastError << "Listen() error ,WSAAsyncSelect failed : " << WSAGetLastError();
		return false;
	}

	if (listen(m_hSocket, nBackLog) == SOCKET_ERROR)
	{
		m_LastError << "listen() error : " << WSAGetLastError();
		return false;
	}

	m_bIsListeningSock = true;

	m_bIsOnline = true;

	return true;
}

bool JuneNet::CSocketItem::Accept(CSocketItem *pNewSockItem, SOCKADDR* pSockAddr /*= nullptr*/, int *pSockAddrLen /*= nullptr*/)
{
	SOCKET s = WSAAccept(m_hSocket, pSockAddr, pSockAddrLen, nullptr, 0);
	if (s == INVALID_SOCKET)
	{
		m_LastError << "Accept() error : Invalid socket.";
		return false;
	}

	if (pNewSockItem == nullptr)
	{
		m_LastError << "Accept() error : SocketItem is nullptr.";
		shutdown(s, SD_BOTH);
		closesocket(s);
		return false;
	}

	if (pNewSockItem->m_hIocp == nullptr)
	{
		m_LastError << "Accept() error : IOCP pointer is nulltpr.";
		shutdown(s, SD_BOTH);
		closesocket(s);
		return false;
	}

	if (pNewSockItem->m_bCreated || pNewSockItem->m_hSocket != INVALID_SOCKET)
	{
		m_LastError << "Accept() error : Socket is aready created.";
		shutdown(s, SD_BOTH);
		closesocket(s);
		return false;
	}

	if (!pNewSockItem->Attach(s, true))
	{
		m_LastError << "Accept() error : Attach error.";
		shutdown(s, SD_BOTH);
		closesocket(s);
		return false;
	}

	if (!pNewSockItem->ReceiveFeedBack(0))
	{
		m_LastError << "Accept() error : ReceiveFeedBack() == false.";
		shutdown(s, SD_BOTH);
		closesocket(s);
		return false;
	}

	pNewSockItem->m_bIsOnline = true;

	return true;
}

bool JuneNet::CSocketItem::Connect(const std::string& ip, int port)
{
	if (m_bIsListeningSock) return false;

	if (!m_bCreated || m_hSocket == INVALID_SOCKET)
	{
		m_LastError << "Connect() error : Socket is invalid or created.";
		return false;
	}

	if (CreateMsgWindow(nullptr) == false)
	{
		m_LastError << "CreateMsgWindow failed.";
		return false;
	}

	if (WSAAsyncSelect(m_hSocket, m_hMsgWnd, UM_FASTSOCKET_NOTIFY, FD_ACCEPT) == SOCKET_ERROR)
	{
		m_LastError << "Listen() error ,WSAAsyncSelect failed : " << WSAGetLastError();
		return false;
	}

	memset(&m_sockAddr, 0, sizeof(m_sockAddr));

	m_sockAddr.sin_family = AF_INET;
	m_sockAddr.sin_addr.s_addr = inet_addr(ip.data());
	m_sockAddr.sin_port = htons(static_cast<u_short>(port));

	if (WSAConnect(m_hSocket, reinterpret_cast<SOCKADDR*>(&m_sockAddr), sizeof(m_sockAddr), nullptr, nullptr, nullptr, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			m_LastError << "Connect() error : Socket connect failed, " << WSAGetLastError();
			return false;
		}
	}

	return true;
}

bool JuneNet::CSocketItem::AfterConnect(bool bConnected)
{
	if (!bConnected)
	{
		return false;
	}

	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_hSocket), m_hIocp, reinterpret_cast<ULONG_PTR>(this), 0) == nullptr)
	{
		m_LastError << "AfterConnect() error : CreateIoCompletionPort() failed, " << GetLastError();
		return false;
	}

	if (!ReceiveFeedBack(0))
	{
		m_LastError << "BlockingConnect() error : ReceiveFeedBack(0) == false";
		return false;
	}

	m_bIsOnline = true;

	return true;
}

bool JuneNet::CSocketItem::BlockingConnect(const std::string& ip, int port)
{
	return true;
}

bool JuneNet::CSocketItem::Attach(SOCKET hSocket, bool bAttachIOCP)
{
	if (m_bCreated || m_hSocket != INVALID_SOCKET)
	{
		m_LastError << "Attach() error : Socket is not created or invalid.";
		return false;
	}

	if (hSocket == INVALID_SOCKET)
	{
		m_LastError << "Attach() error : Socket is invalid.";
		return false;
	}

	if (bAttachIOCP)
	{
		if (m_hIocp == nullptr)
		{
			m_LastError << "Attach() error : IOCP handle is nullptr.";
			return false;
		}

		if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(hSocket), m_hIocp, reinterpret_cast<ULONG_PTR>(this), 0) == nullptr)
		{
			m_LastError << "Attach() error : CreateIoCompletionPort() failed, " << GetLastError();
			return false;
		}
	}

	m_bCreated = true;
	m_hSocket = hSocket;
	m_bClosePosted = false;
	m_bUseForSend = false;

	return true;
}

bool JuneNet::CSocketItem::Send(void *pData, int size)
{
	if (!m_bCreated) return false;

	Lock();

	if (!m_bIsOnline || m_bClosePosted)
	{
		if (IsPostCloseTimeOut() && m_hIocp)
		{
			PostQueuedCompletionStatus(m_hIocp, IOCP_SOCKETCLOSE, reinterpret_cast<ULONG_PTR>(this), &(m_RecvOvl.Overlapped));
		}

		UnLock();
		return false;
	}

	if (m_hSocket == INVALID_SOCKET)
	{
		m_bUseForSend = false;
		UnLock();
		return false;
	}

	if (!m_SendQueue.EnQueue(pData, size))
	{
		m_bUseForSend = false;
		m_SendQueue.Destroy();
		PostClose();
		UnLock();
		return false;
	}

	if (m_bUseForSend || m_bFlushMode)
	{
		UnLock();
		return true;
	}

	m_SendOvl.WsaBuf.buf = m_SendQueue.GetBufPtr();
	m_SendOvl.WsaBuf.len = m_SendQueue.GetBlockSize();

	if (m_SendOvl.WsaBuf.buf == nullptr || m_SendOvl.WsaBuf.len <= 0)
	{
		m_bUseForSend = false;
		UnLock();
		return false;
	}

	DWORD dwSendBytes = 0;

	m_SendOvl.OperationType = OPERATION_SEND;

	if (WSASend(m_hSocket, &m_SendOvl.WsaBuf, 1, &dwSendBytes, 0, &m_SendOvl.Overlapped, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_bUseForSend = false;
			m_SendQueue.Destroy();
			PostClose();
			UnLock();
			return false;
		}
	}

	m_bUseForSend = true;

	UnLock();

	return true;
}

void JuneNet::CSocketItem::SetFlushMode(bool bActive)
{
	m_bFlushMode = bActive;
}

bool JuneNet::CSocketItem::FlushData()
{
	if (!m_bFlushMode) return false;

	Lock();

	if (m_hSocket == INVALID_SOCKET)
	{
		m_bUseForSend = false;
		UnLock();
		return false;
	}

	m_SendOvl.WsaBuf.buf = m_SendQueue.GetBufPtr();
	m_SendOvl.WsaBuf.len = m_SendQueue.GetBlockSize();

	if (m_SendOvl.WsaBuf.buf == nullptr || m_SendOvl.WsaBuf.len <= 0)
	{
		m_bUseForSend = false;

		UnLock();

		return true;
	}

	if (m_bUseForSend)
	{

		UnLock();
		return true;
	}


	DWORD dwSendBytes = 0;
	m_SendOvl.OperationType = OPERATION_SEND;

	if (WSASend(m_hSocket, &(m_SendOvl.WsaBuf), 1, &dwSendBytes, 0, &(m_SendOvl.Overlapped), nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_bUseForSend = FALSE;

			m_SendQueue.Destroy();
			PostClose();
			UnLock();
			return false;
		}
	}


	m_bUseForSend = true;

	UnLock();
	return true;
}

void JuneNet::CSocketItem::OnAccept(int errorCode)
{
	if (errorCode == 0)
	{
		SOCKET s = accept(m_hSocket, nullptr, nullptr);
		if (s != INVALID_SOCKET) closesocket(s);
	}
}

void JuneNet::CSocketItem::OnClose(int errorCode)
{

}

void JuneNet::CSocketItem::OnConnect(int errorCode)
{

}

void JuneNet::CSocketItem::OnReceive(int errorCode)
{

}

bool JuneNet::CSocketItem::SendFeedBack(int transByte)
{
	Lock();

	if (transByte == 0)
	{
		m_bUseForSend = false;
		UnLock();
		return false;
	}


	if (m_hSocket == INVALID_SOCKET)
	{
		m_bUseForSend = false;
		UnLock();
		return false;
	}


	m_SendQueue.DeQueueSize(transByte);

	m_SendOvl.WsaBuf.buf = m_SendQueue.GetBufPtr();
	m_SendOvl.WsaBuf.len = m_SendQueue.GetBlockSize();


	if (m_SendOvl.WsaBuf.buf == NULL || m_SendOvl.WsaBuf.len <= 0)
	{
		m_bUseForSend = false;

		if (m_bClosePosted && m_hIocp)
			PostQueuedCompletionStatus(m_hIocp, IOCP_SOCKETCLOSE, reinterpret_cast<ULONG_PTR>(this), &(m_RecvOvl.Overlapped));

		UnLock();

		return true;
	}


	DWORD dwSendBytes = 0;
	m_SendOvl.OperationType = OPERATION_SEND;

	if (WSASend(m_hSocket, &(m_SendOvl.WsaBuf), 1, &dwSendBytes, 0, &(m_SendOvl.Overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_bUseForSend = false;

			m_SendQueue.Destroy();
			PostClose();
			UnLock();
			return false;
		}
	}


	m_bUseForSend = true;


	UnLock();

	return true;
}

bool JuneNet::CSocketItem::ReceiveFeedBack(int transByte)
{
	Lock();

	if (m_hSocket == INVALID_SOCKET || m_bClosePosted)
	{
		UnLock();
		return false;
	}

	if (transByte > 0)
	{
		if (!m_RecvQueue.EnQueueSize(transByte))
		{
			m_SendQueue.Destroy();
			PostClose();
			UnLock();
			return false;
		}
	}

	m_RecvOvl.WsaBuf.len = m_RecvQueue.GetFreeBufPtr(m_RecvOvl.WsaBuf.buf);



	if (m_RecvOvl.WsaBuf.buf == nullptr || m_RecvOvl.WsaBuf.len <= 0)
	{
		m_SendQueue.Destroy();
		PostClose();
		UnLock();
		return false;
	}

	m_RecvOvl.OperationType = OPERATION_RECV;

	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;
	if (WSARecv(m_hSocket, &(m_RecvOvl.WsaBuf), 1, &dwRecvBytes, &dwFlags,
		&(m_RecvOvl.Overlapped), nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_SendQueue.Destroy();
			PostClose();
			UnLock();
			return FALSE;
		}
	}

	if (transByte > 0)
		OnReceive(0);

	UnLock();

	return true;
}

bool JuneNet::CSocketItem::CloseFeedBack()
{
	Lock();

	if (!m_bCreated || !m_bIsOnline || !m_bClosePosted)
	{
		UnLock();
		return false;
	}

	m_bIsOnline = false;

	if (m_hSocket != INVALID_SOCKET)
	{
		shutdown(m_hSocket, SD_BOTH);
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	}

	UnLock();

	OnClose(0);
	return true;
}

bool JuneNet::CSocketItem::DisconnFeedBack()
{
	Lock();

	m_SendQueue.Destroy();
	PostClose();

	UnLock();

	return true;
}

bool JuneNet::CSocketItem::PostClose()
{
	if (!m_bClosePosted && m_bIsOnline)
	{
		m_bClosePosted = true;
		if (m_hSocket != INVALID_SOCKET)
		{
			m_nCloseTime = timeGetTime();

			if (m_hIocp && m_SendQueue.GetBlockSize() == 0)
				PostQueuedCompletionStatus(m_hIocp, IOCP_SOCKETCLOSE, reinterpret_cast<ULONG_PTR>(this), &(m_RecvOvl.Overlapped));
			return true;
		}
	}
	return false;
}

bool JuneNet::CSocketItem::IsPostCloseTimeOut()
{
	if (m_bClosePosted)
	{
		UINT nowTime = ::timeGetTime();
		if ((nowTime - m_nCloseTime) < 0
			|| (nowTime - m_nCloseTime) > POSTCLOSE_TIMEOUT_TICK)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL JuneNet::CSocketItem::CreateMsgWindow(HWND hWndParent)
{
	if (m_hMsgWnd) return TRUE;

	HWND hwndParent = hWndParent;
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwndParent, GWLP_HINSTANCE);

	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = SockWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(LONG);
	wndclass.hInstance = hInst;
	wndclass.hIcon = NULL;
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = _T("SockWindow");
	wndclass.hIconSm = NULL;

	RegisterClassEx(&wndclass);

	m_hMsgWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		_T("SockWindow"),
		_T("Socket Window"),
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		0,
		0,
		hwndParent,
		NULL,
		hInst,
		NULL);

	if (m_hMsgWnd == NULL)
	{
		m_LastError << "Can\'t create Socket Process Window";
		return FALSE;
	}

	SetWindowLongPtr(m_hMsgWnd, GWLP_USERDATA, (LONG_PTR)this);

	return TRUE;
}

BOOL JuneNet::CSocketItem::DestroyMsgWindow()
{
	if (m_hMsgWnd)
	{
		DestroyWindow(m_hMsgWnd);
		m_hMsgWnd = NULL;
		return TRUE;
	}
	else return FALSE;
}

LRESULT CALLBACK JuneNet::CSocketItem::SockWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
		break;

	case WM_DESTROY:
		break;

	case UM_FASTSOCKET_NOTIFY:
	{
		CSocketItem *pSock = (CSocketItem*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (pSock == NULL) break;

		int ErrorCode = WSAGETSELECTERROR(lParam);

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
		{
			AcceptLock.Lock();
			if (ErrorCode)
			{
				closesocket(wParam);
				pSock->OnAccept(ErrorCode);
			}
			else pSock->OnAccept(0);
			AcceptLock.Unlock();
		} break;

		case FD_CONNECT:
		{
			ConnectLock.Lock();
			if (ErrorCode)
			{
				pSock->AfterConnect(FALSE);
				pSock->OnConnect(ErrorCode);
			}
			else
			{

				if (pSock->AfterConnect(TRUE) == TRUE)
				{
					pSock->OnConnect(0);
				}
				else pSock->OnConnect(-1);
			}
			ConnectLock.Unlock();

		} break;
		}
	}
	break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
