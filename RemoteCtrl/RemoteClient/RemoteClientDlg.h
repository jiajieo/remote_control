
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"
#include "WatchDialog.h"

#define WM_SEND_PACKET (WM_USER+1)//发送数据包的消息ID

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


	//1 查看磁盘分区
	//2 查看指定目录下的文件
	//3 打开文件
	//4 下载文件
	//5 鼠标操作
	//6 远程监控，发送屏幕截图给控制端
	//7 删除文件
	//8 锁机
	//9 解锁
	//1981 测试连接
	int SendPacket(WORD nCmd, BYTE* pData = NULL, size_t nSize = 0, BOOL bAutoClose = TRUE);//发送数据包，确保网络初始化连接正常  bAutoClose=TRUE 套接字自动关闭，表示短连接；否则表示长连接。
private:
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChild(HTREEITEM hTreeSelected);
	void LoadFileInfo();//查看目录信息处理
	void LoadFileCurrent();//刷新目录下的文件
	static unsigned __stdcall threadEntryForDownFile(void* arg);//文件下载线程，在类中添加线程要将线程函数定义为静态的
	void threadDownFile();//文件下载线程
	//屏幕监控
	static unsigned __stdcall threadEntryWatchData(void* arg);//静态函数不能使用this指针
	void threadWatchData();

public://获取成员变量
	bool GetIsFull() const{//该成员函数是一个常量成员函数，不会修改类里的成员变量
		return m_isFull;
	}
	CImage& GetImage(){//这里是将m_image拿到外部用一下，所以用引用，可能会修改m_image变量的值，所以不能加const
		return m_image;
	}
	void SetNoImage(bool isfull = false) {//设为无缓存
		m_isFull = isfull;
	}

private:
	CClientSocket* m_hSocket;//防止多个线程同时用到这个数据，所以将它定义为局部变量
	CStatusDlg m_status;
	CImage m_image;//缓存，提供了增强的位图支持，能够加载和保存JPEG、GIF、BMP的图像。
	bool m_isFull;//是否有缓存,false无缓存,true有缓存 初始化为无缓存
	bool m_isClosed;//监控对话框是否关闭

public:
	CTreeCtrl m_tree;//文件目录树形控件变量
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);//树形控件左键双击事件
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);//树形控件左键单击事件

	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);//列表视图控件右键单击事件
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnOpenFile();
	afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);//定义消息处理函数
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
