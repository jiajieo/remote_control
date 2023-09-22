#pragma once
#include "ClientSocket.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "WatchDialog.h"
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
	LRESULT SendMessageToEn(MSG msg);

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

private:
	//对成员变量初始化
	//m_WatchDlg 初始化为 m_RemoteClientDlg 的地址，说明m_RemoteClientDlg是m_WatchDlg和m_StatusDlg的父类
	CClientControler():m_WatchDlg(&m_RemoteClientDlg), m_StatusDlg(&m_RemoteClientDlg) {
		//启动线程之前进行初始化
		m_thread = INVALID_HANDLE_VALUE;
		m_thrdAddr = -1;
		
	}
	~CClientControler() {
		WaitForSingleObject(m_thread, 100);//线程随着控制类的销毁后等100ms结束
	}
	//消息处理线程
	static unsigned __stdcall threadEntry(void* arg);

	static void releaseInstance();
	class CHelper {
	public:
		CHelper() {
			CClientControler::getInstance();
		}
		~CHelper() {
			CClientControler::releaseInstance();
		}
	};
private:
	static CClientControler* m_instance;
	static CHelper m_helper;
	//客户端父窗口
	CRemoteClientDlg m_RemoteClientDlg;
	//屏幕监控对话框
	CWatchDialog m_WatchDlg;
	//文件下载对话框
	CStatusDlg m_StatusDlg;

	//启动消息处理线程
	HANDLE m_thread;
	unsigned m_thrdAddr;

	typedef LRESULT(CClientControler::* MsgFunc)(UINT nMsg, WPARAM wParam, LPARAM lParam);//
	static std::map<UINT, MsgFunc> m_mapFunc;//消息容器
};

