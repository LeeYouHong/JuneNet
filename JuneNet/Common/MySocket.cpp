#include "MySocket.h"

JuneNet::CMySocket::CMySocket(HWND hWnd, HANDLE hIocp)
	:CSocketItem(hIocp)
	, m_hParentWnd(hWnd)
{
	
}

JuneNet::CMySocket::~CMySocket()
{
	m_hParentWnd = nullptr;
}

void JuneNet::CMySocket::OnAccept(int errorCode)
{
	if (m_hParentWnd != nullptr)
	{
		::PostMessage(m_hParentWnd, UM_SOCK_ACCEPT, errorCode, (LONG)this);
	}
}

void JuneNet::CMySocket::OnClose(int errorCode)
{
	if (m_hParentWnd != nullptr)
	{
		::PostMessage(m_hParentWnd, UM_SOCK_CLOSE, m_UserVal, (LONG)this);
	}

}

void JuneNet::CMySocket::OnConnect(int errorCode)
{
	if (m_hParentWnd != nullptr)
	{
		::PostMessage(m_hParentWnd, UM_SOCK_CONNECT, m_UserVal, (LONG)this);
	}
}

void JuneNet::CMySocket::OnReceive(int errorCode)
{
	if (m_hParentWnd != nullptr)
	{
		::PostMessage(m_hParentWnd, UM_SOCK_RECEIVE, m_UserVal, (LONG)this);
	}
}

bool JuneNet::CMySocket::Close()
{
	return CSocketItem::Close();
}

bool JuneNet::CMySocket::Send(void *pData, int size)
{
	if (pData == nullptr || size <= 0) return false;

	int totalSize = size;

	return CSocketItem::Send(pData, totalSize);
}

bool JuneNet::CMySocket::OnPacketRecv(int packetCnt)
{
	return true;
}

void JuneNet::CMySocket::SetSocketID(int nSid)
{
	m_UserVal = nSid;
}

