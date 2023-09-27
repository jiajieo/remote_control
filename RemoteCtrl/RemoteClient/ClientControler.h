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
	//LRESULT SendMessage(MSG msg);
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

	std::list<CPacket>& GetlistPack() {
		return CClientSocket::getInstance()->GetlistPack();
	}

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
	int SendPacket(WORD nCmd, BYTE* pData = NULL, size_t nSize = 0, BOOL bAutoClose = TRUE);//�������ݰ���ȷ�������ʼ����������  bAutoClose=TRUE �׽����Զ��رգ���ʾ�����ӣ������ʾ�����ӡ� 

	BOOL ConverImage() {
		CClientSocket* pClient = CClientSocket::getInstance();
		//����CImage
		return CTool::Byte2Image(pClient->Getpacket().strData, m_image);
	}

	CImage& GetImage() {
		return m_image;
	}

	//�ļ�����
	int DownLoadFile(CString& ListText, CString& FileDown);
	static unsigned __stdcall threadEntryForDownFile(void* arg);
	void threadDownFile();

	//��Ļ���
	int StartWatch();
	static unsigned __stdcall threadEntryWatch(void* arg);
	void threadWatch();

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
		hThreadW = INVALID_HANDLE_VALUE;
		thraddrW = -1;
		hThreadD = INVALID_HANDLE_VALUE;
		thraddrD = -1;

		//hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		m_isClosed = true;
	}
	~CClientControler() {
		WaitForSingleObject(m_thread, 100);//�߳����ſ���������ٺ��100ms����
		//CloseHandle(hEvent);
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
	//HANDLE hEvent;

	//�ͻ��˸�����
	CRemoteClientDlg m_RemoteClientDlg;
	//��Ļ��ضԻ���
	CWatchDialog m_WatchDlg;
	//�ļ����ضԻ���
	CStatusDlg m_StatusDlg;

	//�ļ�����
	CString m_FilePath;//���ص�·��
	CString m_ListText;//���ص��ļ���
	CString m_FileDown;//Զ�̶˵�·��

	//��Ļ���
	CImage m_image;//ͼ�񻺴棬�ṩ����ǿ��λͼ֧�֣��ܹ����غͱ���JPEG��GIF��BMP��ͼ��
	bool m_isClosed;//�жϼ�ضԻ����Ƿ�ر�

	//����
	static CClientControler* m_instance;
	static CHelper m_helper;

	//������Ϣ�����߳�
	HANDLE m_thread;//���ƶ������߳�
	unsigned m_thrdAddr;
	HANDLE hThreadW;//��Ļ����߳�
	unsigned thraddrW;
	HANDLE hThreadD;//�ļ������߳�
	unsigned thraddrD;

	typedef LRESULT(CClientControler::* MsgFunc)(UINT nMsg, WPARAM wParam, LPARAM lParam);//
	static std::map<UINT, MsgFunc> m_mapFunc;//��Ϣ����
};

