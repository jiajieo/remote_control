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
	m_ListText = ListText;//���ص��ļ���
	m_FileDown = FileDown;//�ļ���Զ�̶˵�·��

	CFileDialog dlg(FALSE, "*", m_ListText.GetString(), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, NULL, &m_RemoteClientDlg);//�����׼Windows�ļ��Ի��򣬵����ڶ����ļ�ɸѡ��Ϊ�գ���Ϊ�������������ļ�������Ҫɸѡ
	if (IDOK == dlg.DoModal()) {//��ʾ�ļ��Ի���
		m_FilePath = dlg.GetPathName();//ѡ������·��

		HANDLE hThreadD = (HANDLE)_beginthreadex(NULL, 0, CClientControler::threadEntryForDownFile, this, 0, &thraddrD);//����threadDownFile �̺߳���Ҫ����Ϊ��̬�ģ�������ʲ���
		if (WaitForSingleObject(hThreadD, 0) == WAIT_TIMEOUT) {
			//��ʾ���ضԻ���
			m_RemoteClientDlg.BeginWaitCursor();
			m_StatusDlg.m_info.SetWindowText("DownLoading...");
			//m_StatusDlg.SetActiveWindow();//�����
			m_StatusDlg.CenterWindow(&m_RemoteClientDlg);//ʹ����������丸������
			m_StatusDlg.ShowWindow(SW_SHOW);
		}

	}
	return 0;
}

unsigned __stdcall CClientControler::threadEntryForDownFile(void* arg)
{
	CClientControler* thiz = (CClientControler*)arg;
	thiz->threadDownFile();
	_endthreadex(0);//��ֹ�߳�
	return 0;
}

void CClientControler::threadDownFile()
{
	/*MSG msg;
		msg.message = WM_SHOW_STATUS;
		SendMessage(msg);*/
	FILE* pFile = fopen(m_FilePath, "wb+");//��Ϊʹ�ö����Ʒ�ʽ��ȡ�ģ��ı�����ͨ�������Ʒ�ʽд�룬dlg.GetPathName()�ļ�������·��
	if (pFile == NULL) {
		AfxMessageBox("�ļ�����ʧ��");
		m_StatusDlg.ShowWindow(SW_HIDE);
		m_RemoteClientDlg.EndWaitCursor();
		return;
	}
	TRACE("FileDown=%s\n", (LPCSTR)m_FileDown);
	do {//��ѭ��ֻ��ִ��һ�Σ��������ֱ���˳�ѭ��
		//int ret = SendPacket(4, (BYTE*)(LPCSTR)FileDown, FileDown.GetLength(), FALSE);//SendPacket����������ⲿ���̲߳��������
		//int ret = OnSendPacket(4 << 1 | 0, (LPARAM)(LPCSTR)FileDown);
		//int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)FileDown);//���ƶ���Ϣ���͵�һ����������
		int ret = SendPacket(4, (BYTE*)m_FileDown.GetBuffer(), m_FileDown.GetLength(), FALSE);
		if (ret < 0) {
			AfxMessageBox("����ʧ��");
			break;
		}
		//�ļ���С
		long long data = *(long long*)Getpacket().strData.c_str();//���ַ���ת����long long����
		if (data == 0) {
			AfxMessageBox("�ļ�����Ϊ0���޷���ȡ�ļ�!");
			break;
		}

		long long nCount = 0;
		while (nCount < data) {
			int ret = Recv();
			if (ret < 0) {
				AfxMessageBox("����ʧ��");
				break;
			}
			size_t len = fwrite(Getpacket().strData.c_str(), 1, Getpacket().strData.size(), pFile);
			nCount += len;
		}
		TRACE("���յ��ļ���С:%d\n", nCount);
		if (nCount == data)
			AfxMessageBox("���سɹ�!", MB_SETFOREGROUND);
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
	WaitForSingleObject(hThreadW, 500);//��ضԻ���رպ�ȴ�500ms�ر��߳�
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
	//ULONGLONG ret = GetTickCount64();//�������������������ĺ�����
	while (m_isClosed == false) {//���رնԻ���
		//if (GetTickCount64() - ret < 50) { //������ÿ��50ms�ڽ�������
		//	Sleep(50 + ret - GetTickCount64());
		//	//Sleep(GetTickCount64() - ret);
		//	ret = GetTickCount64();
		//}

		//int ret = SendPacket(6, NULL, 0);
		if (m_WatchDlg.GetIsFull() == false) {//�޻���ʱ�ڽ������ݵĽ���
			BYTE* Data = NULL;
			int ret = SendMessage(m_RemoteClientDlg, WM_SEND_PACKET, 6 << 1 | 1, (LPARAM)Data);
			/*CPacket pack(6, NULL, NULL);
			MSG msg;
			msg.message = WM_SEND_PACK;
			msg.lParam = (LPARAM)&pack;*/
			//int ret =SendPacket(6);
			if (ret == 6) {//�������ݵ�������
				if (ConverImage() == FALSE) {//���յ�������ת��ΪCImageͼ�񻺴�
					TRACE("��ȡͼƬʧ��!\r\n");
					continue;
				}
				m_WatchDlg.GetIsFull() = true;
			}
			else
				Sleep(1);
		}//������л��棬��ɶҲ���ɣ��ȴ��޻����ٽ������ݣ��Ͳ���Ҫ�ȴ�50ms��
		else
			Sleep(1);
	}
}

//��ΪSendMessage��ȷ���߳�ִ����᷵�أ���������дһ��SendMessage�ӿ�Ŀ�Ľ��̴߳�������Ϣ�ķ��ؽ�����ݳ�����ͨ����PostThreadMessage ��������Ϣ���͵��̵߳���Ϣ���У��ȴ��¼������Ϊ�ź�״̬��������ֵ���ݳ�����
//LRESULT CClientControler::SendMessage(MSG msg)
//{
//	//UUID uuid;//ͨ��Ψһ��ʶ���룬���Ա�ʾ�����ϵͳ�е�ĳ��ʵ�����Դ��ȷ��ȫ��Ψһ��
//	//UuidCreate(&uuid);//�����µ�UUID
//	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//��ʼΪ�ֶ����ź�״̬
//	if (hEvent == NULL)return-2;//����¼����󴴽�ʧ�ܣ�����Ҫ������Ϣ���߳�
//	MsgInfo info(msg);
//	PostThreadMessage(m_thrdAddr, WM_SEND_MESSAGE, (WPARAM)hEvent, (LPARAM)&info);
//	WaitForSingleObject(hEvent, INFINITE);
//	return info.result;
//}

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
