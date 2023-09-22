#pragma once
#include "ClientSocket.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "WatchDialog.h"
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
	LRESULT SendMessageToEn(MSG msg);

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

private:
	//�Գ�Ա������ʼ��
	//m_WatchDlg ��ʼ��Ϊ m_RemoteClientDlg �ĵ�ַ��˵��m_RemoteClientDlg��m_WatchDlg��m_StatusDlg�ĸ���
	CClientControler():m_WatchDlg(&m_RemoteClientDlg), m_StatusDlg(&m_RemoteClientDlg) {
		//�����߳�֮ǰ���г�ʼ��
		m_thread = INVALID_HANDLE_VALUE;
		m_thrdAddr = -1;
		
	}
	~CClientControler() {
		WaitForSingleObject(m_thread, 100);//�߳����ſ���������ٺ��100ms����
	}
	//��Ϣ�����߳�
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
	//�ͻ��˸�����
	CRemoteClientDlg m_RemoteClientDlg;
	//��Ļ��ضԻ���
	CWatchDialog m_WatchDlg;
	//�ļ����ضԻ���
	CStatusDlg m_StatusDlg;

	//������Ϣ�����߳�
	HANDLE m_thread;
	unsigned m_thrdAddr;

	typedef LRESULT(CClientControler::* MsgFunc)(UINT nMsg, WPARAM wParam, LPARAM lParam);//
	static std::map<UINT, MsgFunc> m_mapFunc;//��Ϣ����
};

