
// TimerServerDlg.h : ͷ�ļ�
//

#pragma once


#define GLOBALGAME_TIMER			1000
#define MSGPOOLING_TIMER			1001

// CTimerServerDlg �Ի���
class CTimerServerDlg : public CDialogEx
{
// ����
public:
	CTimerServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TIMERSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


	afx_msg LONG OnServAccept(UINT, LONG);
	afx_msg LONG OnDataClose(UINT, LONG);
	afx_msg LONG OnDataConnect(UINT, LONG);
	afx_msg LONG OnDataReceive(UINT, LONG);

	afx_msg void OnClose();
	afx_msg void OnCancel();

	afx_msg void OnTimer(UINT nIDEvent);

// ʵ��
protected:
	HICON m_hIcon;
	UINT  m_GlobalTimer;
	UINT  m_MsgPollingTimer;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
