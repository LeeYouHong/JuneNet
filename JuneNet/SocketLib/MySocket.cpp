#include "MySocket.h"
#include "../Common/Util.h"

JuneNet::CMySocket::CMySocket(HWND hWnd, HANDLE hIocp)
	:CSocketItem(hIocp)
	, m_hParentWnd(hWnd)
	, m_CloseWaitTimeGap(0)
	, m_CloseDelayTime(0)
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
	ResetMember();

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

bool JuneNet::CMySocket::IsCloseTimeOut()
{
	if (m_CloseDelayTime == 0) return false;

	return JuneCommon::CUtil::CheckTimeOut(timeGetTime(), m_CloseDelayTime, m_CloseWaitTimeGap);
}

bool JuneNet::CMySocket::ReservCloseBeDelay(int delay /*= 500*/)
{
	if (m_CloseDelayTime != 0)
		return false;

	m_CloseDelayTime = timeGetTime();
	m_CloseWaitTimeGap = delay;

	return true;
}

void JuneNet::CMySocket::ResetMember()
{
	m_CloseDelayTime = 0;
	m_CloseWaitTimeGap = 0;
}

