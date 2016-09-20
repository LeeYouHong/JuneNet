/*
FileName: SocketItem.h
Description: Socket
Autor: LeeYouHong
*/

#pragma once

#ifndef __SOCKETITEM_H__
#define __SOCKETITEM_H__

#include <sstream>
#include <WinSock2.h>

#include "SendQueue.h"
#include "RecvQueue.h"

#define UM_FASTSOCKET_NOTIFY	(WM_USER+100)

#define POSTCLOSE_TIMEOUT_TICK	5000

#define IOCP_SOCKETCLOSE  -1	
#define IOCP_THREADCLOSE  -2	


#define OPERATION_SEND  1		
#define OPERATION_RECV  2		
#define OPERATION_CLOSE 3		


struct MYOVERLAPPED
{
	OVERLAPPED   Overlapped;
	WSABUF 		 WsaBuf;
	int			 OperationType;
};


namespace JuneNet
{

	class CMyLock
	{
	public:
		CRITICAL_SECTION m_cs;

		CMyLock()
		{
			InitializeCriticalSection(&m_cs);
		};

		~CMyLock()
		{
			DeleteCriticalSection(&m_cs);
		};

		inline void Lock()
		{
			EnterCriticalSection(&m_cs);
		};

		inline void Unlock()
		{
			LeaveCriticalSection(&m_cs);
		};
	};


	class CSocketItem
	{
	public:
		CSocketItem(HANDLE hIocp);
		virtual ~CSocketItem();

	public:
		void Clear();
		bool Destroy();
		bool Create();
		bool Close();
		bool ReserveClose();
		bool Bind(int nServerPort, const std::string& ip);
		bool Listen(int nBackLog = 5);
		bool Accept(CSocketItem *pNewSockItem, SOCKADDR* pSckAddr = nullptr, int *pSockAddrLen = nullptr);
		bool Connect(const std::string& ip, int port);
		bool AfterConnect(bool bConnected);
		bool BlockingConnect(const std::string& ip, int port);
		bool Attach(SOCKET hSocket, bool bAttachIOCP);
		bool Send(void *pData, int size);

		void SetFlushMode(bool bActive);
		bool FlushData();

		virtual bool IsOnline(){ return m_bIsOnline; }

		virtual void OnAccept(int errorCode);
		virtual void OnClose(int errorCode);
		virtual void OnConnect(int errorCode);
		virtual void OnReceive(int errorCode);

		bool SendFeedBack(int transByte);
		bool ReceiveFeedBack(int transByte);
		bool CloseFeedBack();
		bool DisconnFeedBack();

		BOOL CreateMsgWindow(HWND hWndParent);
		BOOL DestroyMsgWindow();
		static LRESULT CALLBACK SockWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

	public:
		void Lock(){ EnterCriticalSection(&m_CS); }
		void UnLock(){ LeaveCriticalSection(&m_CS); }

	protected:
		bool PostClose();
		bool IsPostCloseTimeOut();

	public:
		HANDLE m_hIocp;
		SOCKET m_hSocket;
		bool   m_bCreated;
		HWND   m_hMsgWnd;

	public:
		bool   m_bIsListeningSock;
		bool   m_bUseForSend;
		bool   m_bClosePosted;
		int    m_nCloseTime;

		DWORD  m_UserVal;
	protected:
		CRITICAL_SECTION    m_CS;
		bool				m_bIsOnline;
		std::stringstream	m_LastError;

		MYOVERLAPPED		m_SendOvl;
		MYOVERLAPPED		m_RecvOvl;

		CSendQueue			m_SendQueue;
		CRecvQueue			m_RecvQueue;

		SOCKADDR_IN			m_sockAddr;
		std::string			m_RemoteIP;

		bool				m_bFlushMode;
		bool				m_bNowFlushing;
	};
}

#endif