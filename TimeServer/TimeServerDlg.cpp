
// TimerServerDlg.cpp : 实现文件
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

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CTimerServerDlg 对话框



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


// CTimerServerDlg 消息处理程序

BOOL CTimerServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTimerServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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

	// 获取时间
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

			// 处理消息包，todo
			//while (true)
			//{
			//}

			// 定时关闭
			if (pSocket->IsCloseTimeOut())
			{
				pSocket->ReserveClose();
			}
		}

	}

	CDialog::OnTimer(nIDEvent);
}
