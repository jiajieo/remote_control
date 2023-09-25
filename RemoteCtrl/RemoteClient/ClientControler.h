#pragma once
#include "ClientSocket.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "WatchDialog.h"
#include "Tool.h"
#include <map>


//WM_USER �� 0X7FFF ���ڶ���ר�ô�����ʹ�õ�˽����Ϣ
#define WM_SEND_PACK (WM_USER+1)//�������ݰ�
#define WM_SEND_DATA (WM_USER+2)//������������
#define WM_DOWN_FILE (WM_USER+3)//�ļ�����
#define WM_SHOW_STATUS (WM_USER+4)//�ļ����ضԻ���
#define WM_START_WATCH (WM_USER+5)//CWatchDialog��Ļ���
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ


class CClientControler
{
public:
	//����
	static CClientControler* getInstance();
	//�����߳�
	int StartThread();//������Ϣ������߳�
	//����
	int Invoke(CWnd*& pMainWnd);
	//����Ϣ�����߳��з�����Ϣ
	LRESULT SendMessage(MSG msg);
	//���������ID��ַ
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

	//1 �鿴���̷���
	//2 �鿴ָ��Ŀ¼�µ��ļ�
	//3 ���ļ�
	//4 �����ļ�
	//5 ������
	//6 Զ�̼�أ�������Ļ��ͼ�����ƶ�
	//7 ɾ���ļ�
	//8 ����
	//9 ����
	//1981 ��������
	int SendPacket(WORD nCmd, BYTE* pData = NULL, size_t nSize = 0, BOOL bAutoClose = TRUE)//�������ݰ���ȷ�������ʼ����������  bAutoClose=TRUE �׽����Զ��رգ���ʾ�����ӣ������ʾ�����ӡ� 
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		int ret = 0;
		if (pClient->InitSocket() == true) {

			pClient->Send(CPacket(nCmd, (const char*)pData, nSize));
			ret = Recv();
			if (bAutoClose)//bAutoCloseĬ��ΪTRUE
				CloseSocket();
		}
		return ret;
	}

	BOOL ConverImage() {
		CClientSocket* pClient = CClientSocket::getInstance();
		//����CImage
		return CTool::Byte2Image(pClient->Getpacket().strData, m_image);
	}

	CImage& GetImage() {
		return m_image;
	}

	//bool GetIsFull() const {//�ó�Ա������һ��������Ա�����������޸�����ĳ�Ա����
	//	return m_isFull;
	//}

	void SetNoImage(bool isfull = false) {//��Ϊ�޻���
		m_isFull = isfull;
	}

	bool& GetIsFull() {
		return m_isFull;
	}

	int DownLoadFile(CString& ListText, CString& FileDown) {

		m_ListText = ListText;//���ص��ļ���
		m_FileDown = FileDown;//�ļ���Զ�̶˵�·��

		CFileDialog dlg(FALSE, "*", m_ListText.GetString(), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, NULL, &m_RemoteClientDlg);//�����׼Windows�ļ��Ի��򣬵����ڶ����ļ�ɸѡ��Ϊ�գ���Ϊ�������������ļ�������Ҫɸѡ
		if (IDOK == dlg.DoModal()) {//��ʾ�ļ��Ի���
			m_FilePath = dlg.GetPathName();//ѡ������·��
			unsigned thraddr;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, CClientControler::threadEntryForDownFile, this, 0, &thraddr);//����threadDownFile �̺߳���Ҫ����Ϊ��̬�ģ�������ʲ���
			if (WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT) {
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
	static unsigned __stdcall threadEntryForDownFile(void* arg)
	{
		CClientControler* thiz = (CClientControler*)arg;
		thiz->threadDownFile();
		_endthreadex(0);//��ֹ�߳�
		return 0;
	}
	void threadDownFile() {
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

	int StartWatch() {
		m_isClosed = false;
		unsigned thraddr;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, threadEntryWatch, this, 0, &thraddr);
		m_WatchDlg.DoModal();
		WaitForSingleObject(hThread, 500);//��ضԻ���رպ�ȴ�500ms�ر��߳�
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
		//ULONGLONG ret = GetTickCount64();//�������������������ĺ�����
		while (m_isClosed == false) {//���رնԻ���
			//if (GetTickCount64() - ret < 50) { //������ÿ��50ms�ڽ�������
			//	Sleep(50 + ret - GetTickCount64());
			//	//Sleep(GetTickCount64() - ret);
			//	ret = GetTickCount64();
			//}

			//int ret = SendPacket(6, NULL, 0);
			if (m_isFull == false) {//�޻���ʱ�ڽ������ݵĽ���
				BYTE* Data = NULL;
				//int ret = SendMessage(WM_SEND_PACKET, 6 << 1 | 1, (LPARAM)Data);
				/*CPacket pack(6, NULL, NULL);
				MSG msg;
				msg.message = WM_SEND_PACK;
				msg.lParam = (LPARAM)&pack;*/
				int ret =SendPacket(6);
				if (ret == 6) {//�������ݵ�������
					if (ConverImage() == FALSE) {//���յ�������ת��ΪCImageͼ�񻺴�
						TRACE("��ȡͼƬʧ��!\r\n");
						continue;
					}
					m_isFull = true;
				}
				else
					Sleep(1);
			}//������л��棬��ɶҲ���ɣ��ȴ��޻����ٽ������ݣ��Ͳ���Ҫ�ȴ�50ms��
			else
				Sleep(1);
		}
	}

private:
	struct MsgInfo {
		MSG msg;//�յ�����Ϣ
		LRESULT result;//���ƺ����ķ���ֵ
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		//���ƹ��캯��Ҫ������
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

	LRESULT OnSendPacket(UINT nMsg, WPARAM wParam, LPARAM lParam);//�������ݰ�
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);//������������
	LRESULT OnDownLoadFile(UINT nMsg, WPARAM wParam, LPARAM lParam);//�ļ�����
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);//�ļ����ضԻ���
	LRESULT OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam);//CWatchDialog��Ļ���

protected:
	//�Գ�Ա������ʼ��
	//m_WatchDlg ��ʼ��Ϊ m_RemoteClientDlg �ĵ�ַ��˵��m_RemoteClientDlg��m_WatchDlg��m_StatusDlg�ĸ���
	CClientControler() :m_StatusDlg(&m_RemoteClientDlg), m_WatchDlg(&m_RemoteClientDlg) {
		//�����߳�֮ǰ���г�ʼ��
		m_thread = INVALID_HANDLE_VALUE;
		m_thrdAddr = -1;

	}
	~CClientControler() {
		WaitForSingleObject(m_thread, 100);//�߳����ſ���������ٺ��100ms����
	}
	//��Ϣ�����߳�
	static unsigned __stdcall threadEntry(void* arg);
	void thread();

	static void releaseInstance();
	class CHelper {
	public:
		CHelper() {
			//��Ϊ����Ĺ��캯���漰������Ի����ָ�����Ը�ģ��Ĺ��첻����һ��ʼ��������Ҫ�ȴ��������App��AfxWinMain()��ʼ����ɺ��ڽ��жԻ���Ĵ������Լ���������ģ��ĵ���������Ի����ָ��
			//CClientControler::getInstance();
		}
		~CHelper() {
			CClientControler::releaseInstance();
		}
	};
private:

	CString m_FilePath;
	//�ͻ��˸�����
	CRemoteClientDlg m_RemoteClientDlg;
	//��Ļ��ضԻ���
	CWatchDialog m_WatchDlg;
	//�ļ����ضԻ���
	CStatusDlg m_StatusDlg;
	CString m_ListText;
	CString m_FileDown;
	//��Ļ��ص�ͼ�񻺴�
	CImage m_image;//���棬�ṩ����ǿ��λͼ֧�֣��ܹ����غͱ���JPEG��GIF��BMP��ͼ��
	bool m_isFull;//�ж�CImage���޻���
	bool m_isClosed;//�жϼ�ضԻ����Ƿ�ر�

	static CClientControler* m_instance;
	static CHelper m_helper;


	//������Ϣ�����߳�
	HANDLE m_thread;
	unsigned m_thrdAddr;

	typedef LRESULT(CClientControler::* MsgFunc)(UINT nMsg, WPARAM wParam, LPARAM lParam);//
	static std::map<UINT, MsgFunc> m_mapFunc;//��Ϣ����
};

