#include "pch.h"
#include "ClientControler.h"
#include "Resource.h"

CClientControler* CClientControler::m_instance = NULL;
CClientControler::CHelper CClientControler::m_helper;
std::map<UINT, CClientControler::MsgFunc> CClientControler::m_mapFunc;

CClientControler* CClientControler::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CClientControler();
		struct {
			UINT nMsg;
			MsgFunc func;
		}data[] = {
			{WM_SEND_PACK,&CClientControler::OnSendPacket},
			{WM_SEND_DATA,&CClientControler::OnSendData},
			{WM_DOWN_FILE,&CClientControler::OnDownLoadFile},
			{WM_SHOW_STATUS,&CClientControler::OnShowStatus},
			{WM_START_WATCH,&CClientControler::OnShowWatch},
			{(UINT)-1,NULL}
		};
		for (int i = 0; data[i].nMsg != -1; i++) {
			m_mapFunc.insert(std::pair<UINT, MsgFunc>(data[i].nMsg, data[i].func));
		}
	}
	return m_instance;
}

int CClientControler::StartThread()
{
	//消息处理的线程一定要在客户端父窗口启动之前就创建。
	m_thread = (HANDLE)_beginthreadex(NULL, 0, CClientControler::threadEntry, this, 0, &m_thrdAddr);
	//文件下载对话框的创建
	m_StatusDlg.Create(IDD_DLG_STATUS, &m_RemoteClientDlg);
	return 0;
}

int CClientControler::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_RemoteClientDlg;
	return m_RemoteClientDlg.DoModal();
}

//因为SendMessage能确保线程执行完会返回，所以这里写一个SendMessage接口目的将线程处理完消息的返回结果传递出来，通过用PostThreadMessage 函数将消息发送到线程的消息队列，等待事件对象变为信号状态，将返回值传递出来。
LRESULT CClientControler::SendMessage(MSG msg)
{
	//UUID uuid;//通用唯一的识别码，可以表示计算机系统中的某个实体和资源，确保全局唯一性
	//UuidCreate(&uuid);//创建新的UUID
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//初始为手动无信号状态
	if (hEvent == NULL)return-2;//如果事件对象创建失败，不需要发送消息给线程
	MsgInfo info(msg);
	PostThreadMessage(m_thrdAddr, WM_SEND_MESSAGE, (WPARAM)hEvent, (LPARAM)&info);
	WaitForSingleObject(hEvent, INFINITE);
	return info.result;
}

LRESULT CClientControler::OnSendPacket(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	//WPARAM占4字节
	//LPARAM 是LONG型，可以表示一个内存地址传递指针，也可以传递整数值
	//if (CClientSocket::getInstance()->InitSocket() == true) {
	//	CString buffer = (LPCTSTR)lParam;
	//	BOOL isClose = wParam & 1;//最后一位如果是1则为TRUE
	//	CPacket pack(wParam>>1, buffer.GetBuffer(), buffer.GetLength());
	//	if (CClientSocket::getInstance()->Send(pack) == false)return -1;
	//	CClientSocket::getInstance()->Recv();
	//	if (isClose) {
	//		CClientSocket::getInstance()->CloseSocket();
	//	}
	//}
	CPacket* pack = (CPacket*)lParam;
	CClientSocket* pClient = CClientSocket::getInstance();
	pClient->Send(*pack);
	return pClient->Recv();
}

LRESULT CClientControler::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	char* str = (char*)wParam;
	CClientSocket* pClient = CClientSocket::getInstance();
	return pClient->Send(str, strlen(str));
}

LRESULT CClientControler::OnDownLoadFile(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CClientControler::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_StatusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientControler::OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_WatchDlg.DoModal();
}

unsigned __stdcall CClientControler::threadEntry(void* arg)
{
	CClientControler* thiz = (CClientControler*)arg;
	thiz->thread();
	_endthreadex(0);
	return 0;
}

void CClientControler::thread()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE) {
			HANDLE hEvent = (HANDLE)msg.wParam;
			MsgInfo* info = (MsgInfo*)msg.lParam;
			std::map<UINT, MsgFunc>::iterator it = m_mapFunc.find(info->msg.message);
			if (it != m_mapFunc.end()) {//在消息容器中找到了该消息
				info->result = (this->*(it->second))(info->msg.message, info->msg.wParam, info->msg.lParam);
			}
			else {
				info->result = -1;
			}
			SetEvent(hEvent);
		}
		else {
			MsgInfo msginfo(msg);
			std::map<UINT, MsgFunc>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {//在消息容器中找到了该消息
				msginfo.result = (this->*(it->second))(msg.message, msg.wParam, msg.lParam);
			}
			else {
				msginfo.result = -1;
			}

		}

	}
}



void CClientControler::releaseInstance()
{
	if (m_instance != NULL) {
		delete m_instance;
		m_instance = NULL;
	}
}
