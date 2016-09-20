
// TimerServerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TimeServer.h"
#include "TimeServerDlg.h"
#include "afxdialogex.h"

#include "SocketManager.h"

#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

JuneNet::CSocketManager SockMan;

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTimerServerDlg �Ի���



CTimerServerDlg::CTimerServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTimerServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_GlobalTimer = 0;
	m_MsgPollingTimer = 0;
}

void CTimerServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTimerServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()

	ON_MESSAGE(UM_SOCK_ACCEPT, OnServAccept)
	ON_MESSAGE(UM_SOCK_CLOSE, OnDataClose)
	ON_MESSAGE(UM_SOCK_CONNECT, OnDataConnect)
	ON_MESSAGE(UM_SOCK_RECEIVE, OnDataReceive)
END_MESSAGE_MAP()


// CTimerServerDlg ��Ϣ�������

BOOL CTimerServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	BOOL bMultiHost = FALSE;
	int iPrimaryHostIndex = 0;
	int iWorkerThreadNum = 1;
	SockMan.Init(this->m_hWnd, iWorkerThreadNum);

	int iServerPort = 1234;
	std::string ip = "0.0.0.0";
	if (SockMan.CreateServerSocket(iServerPort, ip) == FALSE)
	{
		AfxMessageBox(_T("Server ListeningSocket create fail! Applications using the same port number, please make sure there is"));
		return FALSE;
	}

	m_GlobalTimer = SetTimer(GLOBALGAME_TIMER, 1000, nullptr);
	m_MsgPollingTimer = SetTimer(MSGPOOLING_TIMER, 20, nullptr);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CTimerServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CTimerServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTimerServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LONG CTimerServerDlg::OnServAccept(UINT wParam, LONG pSock)
{
	int ErrorCode = wParam;
	if (ErrorCode != 0)
	{

		return 1;
	}

	int hostindex = 0;


	int sid = SockMan.AcceptSocket();

	JuneNet::CMySocket *pSocket = SockMan.GetDataSocketPtr(sid);

	// ��ȡʱ��
	time_t now = time(0);
	std::stringstream ss;
	ss << now << '\n';

	pSocket->Send((void*)ss.str().data(), ss.str().size());

	pSocket->ReservCloseBeDelay(1000);

	return 1;
}

LONG CTimerServerDlg::OnDataClose(UINT wParam, LONG nSockCode)
{
	int sid = wParam;

	JuneNet::CMySocket *pDSock = SockMan.GetDataSocketPtr(sid);
	if (pDSock == NULL)
		return 1;

	SockMan.CloseDataSocket(sid);

	return 1;
}

LONG CTimerServerDlg::OnDataConnect(UINT wParam, LONG pSock)
{
	int sid = wParam;
	JuneNet::CMySocket *pDSock = SockMan.GetDataSocketPtr(sid);
	if (pDSock == NULL) return 1;

	return 1;
}

LONG CTimerServerDlg::OnDataReceive(UINT wParam, LONG pSock)
{
	int sid = wParam;
	JuneNet::CMySocket *pDSock = SockMan.GetDataSocketPtr(sid);
	if (pDSock == NULL) return 1;

	//pDSock->m_RecvQue.DeQue()
	return 1;
}

void CTimerServerDlg::OnClose()
{
	CDialog::OnCancel();
}

void CTimerServerDlg::OnCancel()
{
	CDialog::OnCancel();

	SockMan.Destroy();

	CDialog::OnClose();
}

void CTimerServerDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == GLOBALGAME_TIMER)
	{

	}
	else if (nIDEvent == MSGPOOLING_TIMER)
	{
		JuneNet::CMySocket *pSocket = nullptr;
		for (int sid = 0; sid < MAX_DATA_SOCKET; ++sid)
		{
			pSocket = SockMan.GetDataSocketPtr(sid);

			// ������Ϣ����todo
			//while (true)
			//{
			//}

			// ��ʱ�ر�
			if (pSocket->IsCloseTimeOut())
			{
				pSocket->ReserveClose();
			}
		}

	}

	CDialog::OnTimer(nIDEvent);
}
