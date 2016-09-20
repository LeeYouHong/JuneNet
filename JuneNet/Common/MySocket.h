/*
FileName: MySocket.h
Description: My Socket
Autor: LeeYouHong
*/

#pragma once

#ifndef __MYSOCKET_H__
#define __MYSOCKET_H__

#include "SocketItem.h"

#define UM_SOCK_ACCEPT	(WM_USER+700)			
#define UM_SOCK_CONNECT	(WM_USER+701)			
#define UM_SOCK_RECEIVE	(WM_USER+702)			
#define UM_SOCK_CLOSE	(WM_USER+703)		

#define MAX_DATA_SOCKET 200

namespace JuneNet
{
	class CMySocket : public CSocketItem
	{
	public:
		CMySocket(HWND hWnd, HANDLE hIocp);
		virtual ~CMySocket();

	public:
		virtual void OnAccept(int errorCode);
		virtual void OnClose(int errorCode);
		virtual void OnConnect(int errorCode);
		virtual void OnReceive(int errorCode);

	public:
		bool Close();
		bool ReservCloseBeDelay(int delay = 500);
		bool IsCloseTimeOut();

		bool Send(void *pData, int size);

		bool OnPacketRecv(int packetCnt);
		void SetSocketID(int nSid);

	protected:
		HWND   m_hParentWnd;
	};
}

#endif
