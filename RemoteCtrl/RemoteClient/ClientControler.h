#pragma once
#include "ClientSocket.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "WatchDialog.h"
#include "Tool.h"
#include <map>


//WM_USER 到 0X7FFF 用于定义专用窗口类使用的私人消息
#define WM_SEND_PACK (WM_USER+1)//发送数据包
#define WM_SEND_DATA (WM_USER+2)//单纯发送数据
#define WM_DOWN_FILE (WM_USER+3)//文件下载
#define WM_SHOW_STATUS (WM_USER+4)//文件下载对话框
#define WM_START_WATCH (WM_USER+5)//CWatchDialog屏幕监控
#define WM_SEND_MESSAGE (WM_USER+0x1000)//自定义消息


class CClientControler
{
public:
	//单例
	static CClientControler* getInstance();
	//启动线程
	int StartThread();//启动消息处理的线程
	//启动
	int Invoke(CWnd*& pMainWnd);
	//向消息处理线程中发送消息
	//LRESULT SendMessage(MSG msg);
	//更新网络的ID地址
	void UpdataAddress(DWORD& nIP, int& port) {
		CClientSocket::getInstance()->UpdataAddress(nIP, port);
	}
	bool InitSocket() {
		return CClientSocket::getInstance()->InitSocket();
	}
	int Recv() {
		return CClientSocket::getInstance()->Recv();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}

	CPacket& Getpacket() {
		return CClientSocket::getInstance()->Getpacket();
	}

	std::list<CPacket>& GetlistPack() {
		return CClientSocket::getInstance()->GetlistPack();
	}

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

	BOOL ConverImage() {
		CClientSocket* pClient = CClientSocket::getInstance();
		//存入CImage
		return CTool::Byte2Image(pClient->Getpacket().strData, m_image);
	}

	CImage& GetImage() {
		return m_image;
	}

	//文件下载
	int DownLoadFile(CString& ListText, CString& FileDown);
	static unsigned __stdcall threadEntryForDownFile(void* arg);
	void threadDownFile();

	//屏幕监控
	int StartWatch();
	static unsigned __stdcall threadEntryWatch(void* arg);
	void threadWatch();

private:
	struct MsgInfo {
		MSG msg;//收到的消息
		LRESULT result;//控制函数的返回值
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		//复制构造函数要用引用
		MsgInfo(const MsgInfo& msginfo) {
			result = msginfo.result;
			memcpy(&msg, &msginfo.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& msginfo) {
			if (this != &msginfo) {
				result = msginfo.result;
				memcpy(&msg, &msginfo.msg, sizeof(MSG));
			}
			else
				return *this;
		}
	};

	LRESULT OnSendPacket(UINT nMsg, WPARAM wParam, LPARAM lParam);//发送数据包
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);//单纯发送数据
	LRESULT OnDownLoadFile(UINT nMsg, WPARAM wParam, LPARAM lParam);//文件下载
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);//文件下载对话框
	LRESULT OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam);//CWatchDialog屏幕监控

protected:
	//对成员变量初始化
	//m_WatchDlg 初始化为 m_RemoteClientDlg 的地址，说明m_RemoteClientDlg是m_WatchDlg和m_StatusDlg的父类
	CClientControler() :m_StatusDlg(&m_RemoteClientDlg), m_WatchDlg(&m_RemoteClientDlg) {
		//启动线程之前进行初始化
		m_thread = INVALID_HANDLE_VALUE;
		m_thrdAddr = -1;
		hThreadW = INVALID_HANDLE_VALUE;
		thraddrW = -1;
		hThreadD = INVALID_HANDLE_VALUE;
		thraddrD = -1;

		//hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		m_isClosed = true;
	}
	~CClientControler() {
		WaitForSingleObject(m_thread, 100);//线程随着控制类的销毁后等100ms结束
		//CloseHandle(hEvent);
	}
	//消息处理线程
	static unsigned __stdcall threadEntry(void* arg);
	void thread();

	static void releaseInstance();
	class CHelper {
	public:
		CHelper() {
			//因为该类的构造函数涉及到父类对话框的指向，所以该模块的构造不能在一开始就启动，要等待程序入口App的AfxWinMain()初始化完成后在进行对话框的创建，以及启动控制模块的单例，父类对话框的指向。
			//CClientControler::getInstance();
		}
		~CHelper() {
			CClientControler::releaseInstance();
		}
	};
private:
	//HANDLE hEvent;

	//客户端父窗口
	CRemoteClientDlg m_RemoteClientDlg;
	//屏幕监控对话框
	CWatchDialog m_WatchDlg;
	//文件下载对话框
	CStatusDlg m_StatusDlg;

	//文件下载
	CString m_FilePath;//本地的路径
	CString m_ListText;//下载的文件名
	CString m_FileDown;//远程端的路径

	//屏幕监控
	CImage m_image;//图像缓存，提供了增强的位图支持，能够加载和保存JPEG、GIF、BMP的图像。
	bool m_isClosed;//判断监控对话框是否关闭

	//单例
	static CClientControler* m_instance;
	static CHelper m_helper;

	//启动消息处理线程
	HANDLE m_thread;//控制端启动线程
	unsigned m_thrdAddr;
	HANDLE hThreadW;//屏幕监控线程
	unsigned thraddrW;
	HANDLE hThreadD;//文件下载线程
	unsigned thraddrD;

	typedef LRESULT(CClientControler::* MsgFunc)(UINT nMsg, WPARAM wParam, LPARAM lParam);//
	static std::map<UINT, MsgFunc> m_mapFunc;//消息容器
};

