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
	LRESULT SendMessage(MSG msg);
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

	

	/*int SendPacket(const CPacket& pack) {
		CClientSocket* pClient = CClientSocket::getInstance();
		pClient->Send(pack);
	}*/

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
	int SendPacket(WORD nCmd, BYTE* pData = NULL, size_t nSize = 0, BOOL bAutoClose = TRUE)//发送数据包，确保网络初始化连接正常  bAutoClose=TRUE 套接字自动关闭，表示短连接；否则表示长连接。 
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		int ret = 0;
		if (pClient->InitSocket() == true) {

			pClient->Send(CPacket(nCmd, (const char*)pData, nSize));
			ret = Recv();
			if (bAutoClose)//bAutoClose默认为TRUE
				CloseSocket();
		}
		return ret;
	}

	BOOL ConverImage() {
		CClientSocket* pClient = CClientSocket::getInstance();
		//存入CImage
		return CTool::Byte2Image(pClient->Getpacket().strData, m_image);
	}

	CImage& GetImage() {
		return m_image;
	}

	//bool GetIsFull() const {//该成员函数是一个常量成员函数，不会修改类里的成员变量
	//	return m_isFull;
	//}

	void SetNoImage(bool isfull = false) {//设为无缓存
		m_isFull = isfull;
	}

	bool& GetIsFull() {
		return m_isFull;
	}

	int DownLoadFile(CString& ListText, CString& FileDown) {

		m_ListText = ListText;//下载的文件名
		m_FileDown = FileDown;//文件在远程端的路径

		CFileDialog dlg(FALSE, "*", m_ListText.GetString(), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, NULL, &m_RemoteClientDlg);//构造标准Windows文件对话框，倒数第二项文件筛选器为空，因为可以下载任意文件，不需要筛选
		if (IDOK == dlg.DoModal()) {//显示文件对话框
			m_FilePath = dlg.GetPathName();//选择下载路径
			unsigned thraddr;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, CClientControler::threadEntryForDownFile, this, 0, &thraddr);//这里threadDownFile 线程函数要定义为静态的，否则访问不了
			if (WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT) {
				//显示下载对话框
				m_RemoteClientDlg.BeginWaitCursor();
				m_StatusDlg.m_info.SetWindowText("DownLoading...");
				//m_StatusDlg.SetActiveWindow();//激活窗口
				m_StatusDlg.CenterWindow(&m_RemoteClientDlg);//使窗口相对于其父级居中
				m_StatusDlg.ShowWindow(SW_SHOW);
			}

		}
		return 0;
	}
	static unsigned __stdcall threadEntryForDownFile(void* arg)
	{
		CClientControler* thiz = (CClientControler*)arg;
		thiz->threadDownFile();
		_endthreadex(0);//终止线程
		return 0;
	}
	void threadDownFile() {
		/*MSG msg;
		msg.message = WM_SHOW_STATUS;
		SendMessage(msg);*/
		FILE* pFile = fopen(m_FilePath, "wb+");//因为使用二进制方式读取的，文本可以通过二进制方式写入，dlg.GetPathName()文件的完整路径
		if (pFile == NULL) {
			AfxMessageBox("文件创建失败");
			m_StatusDlg.ShowWindow(SW_HIDE);
			m_RemoteClientDlg.EndWaitCursor();
			return;
		}
		TRACE("FileDown=%s\n", (LPCSTR)m_FileDown);
		do {//该循环只会执行一次，方便出错直接退出循环
			//int ret = SendPacket(4, (BYTE*)(LPCSTR)FileDown, FileDown.GetLength(), FALSE);//SendPacket这个函数是外部的线程不能随便用
			//int ret = OnSendPacket(4 << 1 | 0, (LPARAM)(LPCSTR)FileDown);
			//int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)FileDown);//将制定消息发送到一个或多个窗口
			int ret = SendPacket(4, (BYTE*)m_FileDown.GetBuffer(), m_FileDown.GetLength(), FALSE);
			if (ret < 0) {
				AfxMessageBox("下载失败");
				break;
			}
			//文件大小
			long long data = *(long long*)Getpacket().strData.c_str();//将字符串转换成long long整型
			if (data == 0) {
				AfxMessageBox("文件长度为0，无法读取文件!");
				break;
			}

			long long nCount = 0;
			while (nCount < data) {
				int ret = Recv();
				if (ret < 0) {
					AfxMessageBox("传输失败");
					break;
				}
				size_t len = fwrite(Getpacket().strData.c_str(), 1, Getpacket().strData.size(), pFile);
				nCount += len;
			}
			TRACE("接收到文件大小:%d\n", nCount);
			if (nCount == data)
				AfxMessageBox("下载成功!", MB_SETFOREGROUND);
		} while (false);
		fclose(pFile);
		CloseSocket();
		m_StatusDlg.ShowWindow(SW_HIDE);
		m_RemoteClientDlg.EndWaitCursor();
	}

	int StartWatch() {
		m_isClosed = false;
		unsigned thraddr;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, threadEntryWatch, this, 0, &thraddr);
		m_WatchDlg.DoModal();
		WaitForSingleObject(hThread, 500);//监控对话框关闭后等待500ms关闭线程
		m_isClosed = true;
		return 0;
	}

	static unsigned __stdcall threadEntryWatch(void* arg) {
		CClientControler* thiz = (CClientControler*)arg;
		thiz->threadWatch();
		_endthreadex(0);
		return 0;
	}

	void threadWatch() {
		Sleep(50);
		//ULONGLONG ret = GetTickCount64();//检索自启动以来经过的毫秒数
		while (m_isClosed == false) {//不关闭对话框
			//if (GetTickCount64() - ret < 50) { //这里是每过50ms在接收数据
			//	Sleep(50 + ret - GetTickCount64());
			//	//Sleep(GetTickCount64() - ret);
			//	ret = GetTickCount64();
			//}

			//int ret = SendPacket(6, NULL, 0);
			if (m_isFull == false) {//无缓存时在进行数据的接收
				BYTE* Data = NULL;
				//int ret = SendMessage(WM_SEND_PACKET, 6 << 1 | 1, (LPARAM)Data);
				/*CPacket pack(6, NULL, NULL);
				MSG msg;
				msg.message = WM_SEND_PACK;
				msg.lParam = (LPARAM)&pack;*/
				int ret =SendPacket(6);
				if (ret == 6) {//更新数据到缓存器
					if (ConverImage() == FALSE) {//将收到的数据转换为CImage图像缓存
						TRACE("获取图片失败!\r\n");
						continue;
					}
					m_isFull = true;
				}
				else
					Sleep(1);
			}//如果还有缓存，就啥也不干，等待无缓存再接受数据，就不需要等待50ms了
			else
				Sleep(1);
		}
	}

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

	}
	~CClientControler() {
		WaitForSingleObject(m_thread, 100);//线程随着控制类的销毁后等100ms结束
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

	CString m_FilePath;
	//客户端父窗口
	CRemoteClientDlg m_RemoteClientDlg;
	//屏幕监控对话框
	CWatchDialog m_WatchDlg;
	//文件下载对话框
	CStatusDlg m_StatusDlg;
	CString m_ListText;
	CString m_FileDown;
	//屏幕监控的图像缓存
	CImage m_image;//缓存，提供了增强的位图支持，能够加载和保存JPEG、GIF、BMP的图像。
	bool m_isFull;//判断CImage有无缓存
	bool m_isClosed;//判断监控对话框是否关闭

	static CClientControler* m_instance;
	static CHelper m_helper;


	//启动消息处理线程
	HANDLE m_thread;
	unsigned m_thrdAddr;

	typedef LRESULT(CClientControler::* MsgFunc)(UINT nMsg, WPARAM wParam, LPARAM lParam);//
	static std::map<UINT, MsgFunc> m_mapFunc;//消息容器
};

