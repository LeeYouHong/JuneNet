
// TimerServerDlg.h : 头文件
//

#pragma once


#define GLOBALGAME_TIMER			1000
#define MSGPOOLING_TIMER			1001

// CTimerServerDlg 对话框
class CTimerServerDlg : public CDialogEx
{
// 构造
public:
	CTimerServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TIMERSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	afx_msg LONG OnServAccept(UINT, LONG);
	afx_msg LONG OnDataClose(UINT, LONG);
	afx_msg LONG OnDataConnect(UINT, LONG);
	afx_msg LONG OnDataReceive(UINT, LONG);

	afx_msg void OnClose();
	afx_msg void OnCancel();

	afx_msg void OnTimer(UINT nIDEvent);

// 实现
protected:
	HICON m_hIcon;
	UINT  m_GlobalTimer;
	UINT  m_MsgPollingTimer;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
