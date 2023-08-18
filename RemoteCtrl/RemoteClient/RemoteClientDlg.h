
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"

// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnConnect();
	// 被控端的IP地址
	DWORD m_servaddress;
	// 被控端的端口号
	CString m_port;
	afx_msg void OnBnClickedBtnViewfile();

private:
	int SendPacket(WORD nCmd,BYTE* pData=NULL, size_t nSize=0);//发送数据包

private:
	CClientSocket* m_hSocket;
public:
	CTreeCtrl m_tree;
};
