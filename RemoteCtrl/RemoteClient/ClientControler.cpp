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

int CClientControler::SendPacket(WORD nCmd, BYTE* pData, size_t nSize, BOOL bAutoClose, std::list<CPacket>* plstPack)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	//pClient->GetlistPack().push_back(CPacket(nCmd, (const char*)pData, nSize));
	//ResetEvent(hEvent);
	//HANDLE m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//pClient->SendPacket(m_hEvent,bAutoClose);
	//int ret;
	//if (pClient->InitSocket() == true) {
	HANDLE m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	pClient->SendPacket(CPacket(nCmd, (char*)pData, nSize, m_hEvent), *plstPack);
	if (plstPack->size() > 0) {
		return plstPack->front().sCmd;
	}
	//}
	CloseHandle(m_hEvent);
	return -1;
}

int CClientControler::DownLoadFile(CString& ListText, CString& FileDown)
{
	m_ListText = ListText;//下载的文件名
	m_FileDown = FileDown;//文件在远程端的路径

	CFileDialog dlg(FALSE, "*", m_ListText.GetString(), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, NULL, &m_RemoteClientDlg);//构造标准Windows文件对话框，倒数第二项文件筛选器为空，因为可以下载任意文件，不需要筛选
	if (IDOK == dlg.DoModal()) {//显示文件对话框
		m_FilePath = dlg.GetPathName();//选择下载路径

		HANDLE hThreadD = (HANDLE)_beginthreadex(NULL, 0, CClientControler::threadEntryForDownFile, this, 0, &thraddrD);//这里threadDownFile 线程函数要定义为静态的，否则访问不了
		if (WaitForSingleObject(hThreadD, 0) == WAIT_TIMEOUT) {
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

unsigned __stdcall CClientControler::threadEntryForDownFile(void* arg)
{
	CClientControler* thiz = (CClientControler*)arg;
	thiz->threadDownFile();
	_endthreadex(0);//终止线程
	return 0;
}

void CClientControler::threadDownFile()
{
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

int CClientControler::StartWatch()
{
	m_isClosed = false;
	hThreadW = (HANDLE)_beginthreadex(NULL, 0, threadEntryWatch, this, 0, &thraddrW);
	m_WatchDlg.DoModal();
	WaitForSingleObject(hThreadW, 500);//监控对话框关闭后等待500ms关闭线程
	m_isClosed = true;
	return 0;
}

unsigned __stdcall CClientControler::threadEntryWatch(void* arg)
{
	CClientControler* thiz = (CClientControler*)arg;
	thiz->threadWatch();
	_endthreadex(0);
	return 0;
}

void CClientControler::threadWatch()
{
	Sleep(50);
	//ULONGLONG ret = GetTickCount64();//检索自启动以来经过的毫秒数
	while (m_isClosed == false) {//不关闭对话框
		//if (GetTickCount64() - ret < 50) { //这里是每过50ms在接收数据
		//	Sleep(50 + ret - GetTickCount64());
		//	//Sleep(GetTickCount64() - ret);
		//	ret = GetTickCount64();
		//}

		//int ret = SendPacket(6, NULL, 0);
		if (m_WatchDlg.GetIsFull() == false) {//无缓存时在进行数据的接收
			BYTE* Data = NULL;
			int ret = SendMessage(m_RemoteClientDlg, WM_SEND_PACKET, 6 << 1 | 1, (LPARAM)Data);
			/*CPacket pack(6, NULL, NULL);
			MSG msg;
			msg.message = WM_SEND_PACK;
			msg.lParam = (LPARAM)&pack;*/
			//int ret =SendPacket(6);
			if (ret == 6) {//更新数据到缓存器
				if (ConverImage() == FALSE) {//将收到的数据转换为CImage图像缓存
					TRACE("获取图片失败!\r\n");
					continue;
				}
				m_WatchDlg.GetIsFull() = true;
			}
			else
				Sleep(1);
		}//如果还有缓存，就啥也不干，等待无缓存再接受数据，就不需要等待50ms了
		else
			Sleep(1);
	}
}

//因为SendMessage能确保线程执行完会返回，所以这里写一个SendMessage接口目的将线程处理完消息的返回结果传递出来，通过用PostThreadMessage 函数将消息发送到线程的消息队列，等待事件对象变为信号状态，将返回值传递出来。
//LRESULT CClientControler::SendMessage(MSG msg)
//{
//	//UUID uuid;//通用唯一的识别码，可以表示计算机系统中的某个实体和资源，确保全局唯一性
//	//UuidCreate(&uuid);//创建新的UUID
//	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//初始为手动无信号状态
//	if (hEvent == NULL)return-2;//如果事件对象创建失败，不需要发送消息给线程
//	MsgInfo info(msg);
//	PostThreadMessage(m_thrdAddr, WM_SEND_MESSAGE, (WPARAM)hEvent, (LPARAM)&info);
//	WaitForSingleObject(hEvent, INFINITE);
//	return info.result;
//}

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
