/*
FileName: SocketManager.h
Description: Socket Manager
Autor: LeeYouHong
*/

#pragma once

#ifndef __SOCKETMANAGER_H__
#define __SOCKETMANAGER_H__

#include "MySocket.h"
#include "IocpThread.h"

#include <list>
#include <string>

namespace JuneNet
{

	class CSocketManager
	{
	public:
		CSocketManager();
		virtual ~CSocketManager();

	public:
		bool Init(HWND hWnd, int threadNum = 0);

		void Destroy();

		bool CreateServerSocket(int port, std::string& ip);
		bool CloseServerSocket();

		CMySocket* GetAcceptibleSocket(int& nIdx);
		CMySocket* GetDataSocketPtr(int sid);

		int AcceptSocket();
		int ConnectSocket(const std::string& ip, int port);

		bool SendData(CMySocket* pSock, void *pData, int size);

		bool ReservClose(int sid);
		bool CloseDataSocket(int sid);
		bool CloseDataSocket(CMySocket *pSocket);

		bool DelKeepSocket(CMySocket *pSocket);
		bool AddKeepSocket(CMySocket *pSocket, bool bConnect=false);
		bool RearrangeKeepSocket(CMySocket *pSocket);

		bool OnPacketRecv(CMySocket *pSocket, int nPacketCnt, int nSize);

	private:
		HINSTANCE m_hWs2Dll;
		CIocpThread m_IocpThread;
		HWND m_hWindows;
		HANDLE m_hIOCP;
		CMySocket *m_SocketList[MAX_DATA_SOCKET];
		CMySocket *m_ServerSocket;

		int m_nLastAcceptIdx;

		std::list<CMySocket*> m_KeepList;
	};
}

#endif
