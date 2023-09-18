#pragma once
#include <list>
#include "CPacket.h"
#include "Tool.h"

typedef void(*SOCKET_CALLBACK)(void* arg, int cmd, std::list<CPacket>& lstPacket,CPacket&);//����һ������ָ������

#define BUFFER_SIZE 409600//�������ݰ��Ļ�������С
class CServerSocket
{
public:
	static CServerSocket* getInstance() {//��̬����û��thisָ�룬�޷�ֱ�ӷ��ʳ�Ա��������̬��ȫ�ֵĸ�thisָ�����޹صģ���ȫ��������thisָ�������������̬����
		if (m_instance == NULL) {
			m_instance = new CServerSocket();//new ������delete��һ��Ҫ��գ������޷�ִ������
		}
		return m_instance;
	}

	int Run(SOCKET_CALLBACK callback, void* arg,short port=6000) {//arg�Ǵ���CCommand�����
		std::list<CPacket> lstPacket;//����һ�����������洢����õ����ݡ�
		// 2.�׽��ֵĴ���������
		if (InitSocket(port) == false)return -1;
		int count = 0;// 3.�ṩ3�����ӻ��᣻�����ǰ��ĳ�ʼ�������ʹ����׽��ֲ����Ҳ����أ�ֻ��������ı���صȴ����Ӳ���
		while (true) {
			if (Accept() == false) {
				if (count >= 2) {
					return -2;
				}
				count++;
			}
			//���ӳɹ��������
			//��������
			int ret = Recv();
			callback(arg, ret,lstPacket,m_pack);
			while (lstPacket.size() > 0) {
				Sleep(1);
				Send(lstPacket.front());
				lstPacket.pop_front();
			}
			//Զ�̿��Ʋ��ö�����
			CloseClient();
		}
		/*m_callback = callback;
		m_arg = arg;*/
	}

protected:
	bool InitSocket(short port) {//�׽��ִ���������
		//1 �����׽���
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);//AF_INET��PF_INET����û̫�����⣬��ָ���Ͻ���socketָ��Э��Ӧ����PF_INET�����õ�ַʱ��AF_INET������ʹ��TCP������UDP����ΪҪ���͵������ǿ��ŵ�
		if (INVALID_SOCKET == m_sockSrv) {
			TRACE("sockSrv error=%d\n", WSAGetLastError());
			return false;
		}
		//2 bind
		SOCKADDR_IN addrSrv;
		memset(&addrSrv, 0, sizeof(addrSrv));//����Ľṹ��Ҫ���
		addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//��������IP��ַ��Ϣ��htonl(INADDR_ANY)�ڷ����ָ����������IP��ַ��Ϣ; INADDR_ANY ���е�IP��ȥ��������֤�ͻ��˿�����������
		addrSrv.sin_family = AF_INET;//����ĵ�ַ��,IP����IPv4
		addrSrv.sin_port = htons(port);//��������˿ں�
		if (SOCKET_ERROR == bind(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) {
			TRACE("bind error=%d\n", WSAGetLastError());
			return false;
		}
		//3 listen
		if (SOCKET_ERROR == listen(m_sockSrv, 1)) {//Զ�̼����һ��һ�ģ����Լ���1������
			TRACE("listen error=%d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool Accept() {
		//4 accept
		SOCKADDR_IN addrCli;
		int len = sizeof(addrCli);
		m_sockcli = accept(m_sockSrv, (SOCKADDR*)&addrCli, &len);
		if (m_sockcli == INVALID_SOCKET)
			return false;
		return true;
		//closesocket(m_sockcli);
	}
	int Recv() {
		if (m_sockcli == INVALID_SOCKET)return -1;//����ֻ�����������û�취���շ��͵�
		char buffer[BUFFER_SIZE] = "";
		//char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));
		size_t index = 0;//���ݰ����ն�λ
		//���ݽ��մ���
		while (true) {
			size_t len = recv(m_sockcli, buffer + index, BUFFER_SIZE - index, 0);
			if (len == SOCKET_ERROR) {
				TRACE("recv error=%d\n", WSAGetLastError());
				return -1;
			}
			index += len;
			len = index;
			m_pack = CPacket((BYTE*)buffer, len);//��������
			//Dump((BYTE*)m_pack.Data(), m_pack.Size());
			if (len > 0) {//�����ɹ�
				memmove(buffer, buffer + len, BUFFER_SIZE - len);//��buffer len����������Ƶ���ͷ����������memmove���������Ƶ���һ��������
				index -= len;
				return m_pack.sCmd;//���յ��������
			}
			else {
				memset(buffer, 0, BUFFER_SIZE);
				index = 0;
			}
		}
		return -1;
	}

	CPacket Getpacket() {//��ȡ���ݰ�������֤
		return m_pack;
	}

	bool Send(const char* pData, int nSize) {
		if (m_sockcli == INVALID_SOCKET)return false;
		return send(m_sockcli, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {//�������� ��һ�����Է������ݰ���Send����
		if (m_sockcli == INVALID_SOCKET)return false;
		TRACE("serv datalen=%d\n", pack.Size());
		return send(m_sockcli, pack.Data(), pack.Size(), 0) > 0;//��CPacket��ת��const char*��(const char*)&pack
	}

	bool GetFilePath(std::string& strPath) {//���ƶ���Ҫ��ȡ�ļ�·����Ȩ��
		if ((m_pack.sCmd == 2) || (m_pack.sCmd == 3) || (m_pack.sCmd == 4) || (m_pack.sCmd == 7)) {//��������2��3�����Ի�ȡҪ���ʵ�·��
			strPath = m_pack.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEV& mouse) {//����ѡ����Ӧ����ָ�򴫽��������ĵ�ַ
		if (m_pack.sCmd == 5) {//����һ���ṹ������ȡ����һЩ���ԣ����磺������Ҽ���������˫�����ƶ���
			memcpy(&mouse, m_pack.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	void CloseClient() {
		if (m_sockcli != INVALID_SOCKET) {
			closesocket(m_sockcli);
			m_sockcli = INVALID_SOCKET;
		}
	}

private://����������ĸ��ƣ���ֵ���캯����ҪдΪ˽�еģ��������ⲿ�Ľ��й���
	CServerSocket& operator = (const CServerSocket& ss) {
		m_sockcli = ss.m_sockcli;
		m_sockSrv = ss.m_sockSrv;
	}
	CServerSocket(const CServerSocket& ss) {
		m_sockcli = ss.m_sockcli;
		m_sockSrv = ss.m_sockSrv;
	}
	CServerSocket() {
		m_sockcli = INVALID_SOCKET;
		m_sockSrv = INVALID_SOCKET;
		if (WSAServerInit() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ�������׽��ֿ�"), _T("�׽��ֳ�ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);//�ر������ļ�����ֹ����ִ�еĽ���
		}
	}
	~CServerSocket() {
		closesocket(m_sockSrv);
		WSACleanup();//���ж���û��ִ����������ʱҪ�ѵȴ����շ��Ͳ���ע�͵�
	}

	BOOL WSAServerInit() {
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(1, 1);//Ϊ�˾������ڸ�����������У�������õ�һ���1.1�汾��
		// ��ʼ���׽��ֿ�
		int err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0)
		{
			return FALSE;
		}
		if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)//���� MAKEWORD(BYTE bLow,BYTE bHigh) ��İ汾���й�(��λ�ֽں͸�λ�ֽ�)
		{
			WSACleanup();
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance() {//����m_instanceҪ��ɾ�̬��
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	class CHelper {//��ΪCServerSocket�������û�����õ��������ڶ���һ����������CServerSocket����������
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};

private:
	static CServerSocket* m_instance;//������Ϊ������ľ�̬����Ҫ���ʵ�ʵ��������Ҫ��Ϊ��̬��
	static CHelper m_helper;
	SOCKET m_sockSrv;//�����׽���;�Ǿ�̬�����ڹ��캯�����ʼ��
	SOCKET m_sockcli;
	CPacket m_pack;//���յ����ݰ�����

	SOCKET_CALLBACK m_callback;
	void* m_arg;
};

//extern CServerSocket server;