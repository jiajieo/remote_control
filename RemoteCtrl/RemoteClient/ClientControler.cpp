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
	//��Ϣ������߳�һ��Ҫ�ڿͻ��˸���������֮ǰ�ʹ�����
	m_thread = (HANDLE)_beginthreadex(NULL, 0, CClientControler::threadEntry, this, 0, &m_thrdAddr);
	//�ļ����ضԻ���Ĵ���
	m_StatusDlg.Create(IDD_DLG_STATUS, &m_RemoteClientDlg);
	return 0;
}

int CClientControler::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_RemoteClientDlg;
	return m_RemoteClientDlg.DoModal();
}

//��ΪSendMessage��ȷ���߳�ִ����᷵�أ���������дһ��SendMessage�ӿ�Ŀ�Ľ��̴߳�������Ϣ�ķ��ؽ�����ݳ�����ͨ����PostThreadMessage ��������Ϣ���͵��̵߳���Ϣ���У��ȴ��¼������Ϊ�ź�״̬��������ֵ���ݳ�����
LRESULT CClientControler::SendMessage(MSG msg)
{
	//UUID uuid;//ͨ��Ψһ��ʶ���룬���Ա�ʾ�����ϵͳ�е�ĳ��ʵ�����Դ��ȷ��ȫ��Ψһ��
	//UuidCreate(&uuid);//�����µ�UUID
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//��ʼΪ�ֶ����ź�״̬
	if (hEvent == NULL)return-2;//����¼����󴴽�ʧ�ܣ�����Ҫ������Ϣ���߳�
	MsgInfo info(msg);
	PostThreadMessage(m_thrdAddr, WM_SEND_MESSAGE, (WPARAM)hEvent, (LPARAM)&info);
	WaitForSingleObject(hEvent, INFINITE);
	return info.result;
}

LRESULT CClientControler::OnSendPacket(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	//WPARAMռ4�ֽ�
	//LPARAM ��LONG�ͣ����Ա�ʾһ���ڴ��ַ����ָ�룬Ҳ���Դ�������ֵ
	//if (CClientSocket::getInstance()->InitSocket() == true) {
	//	CString buffer = (LPCTSTR)lParam;
	//	BOOL isClose = wParam & 1;//���һλ�����1��ΪTRUE
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
			if (it != m_mapFunc.end()) {//����Ϣ�������ҵ��˸���Ϣ
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
			if (it != m_mapFunc.end()) {//����Ϣ�������ҵ��˸���Ϣ
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
