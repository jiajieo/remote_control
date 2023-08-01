#pragma once
class CServerSocket
{
public:
	static CServerSocket* getInstance() {//��̬����û��thisָ�룬�޷�ֱ�ӷ��ʳ�Ա��������̬��ȫ�ֵĸ�thisָ�����޹صģ���ȫ��������thisָ�������������̬����
		if (m_instance == NULL) {
			m_instance = new CServerSocket();//new ������delete��һ��Ҫ��գ������޷�ִ������
		}
		return m_instance;
	}

	bool InitSocket() {//�׽��ִ���������
		//1 �����׽���
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);//AF_INET��PF_INET����û̫�����⣬��ָ���Ͻ���socketָ��Э��Ӧ����PF_INET�����õ�ַʱ��AF_INET������ʹ��TCP������UDP����ΪҪ���͵������ǿ��ŵ�
		if (INVALID_SOCKET == m_sockSrv) {
			printf("sockSrv error=%d\n", WSAGetLastError());
			return false;
		}
		//2 bind
		SOCKADDR_IN addrSrv;
		memset(&addrSrv, 0, sizeof(addrSrv));//����Ľṹ��Ҫ���
		addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//��������IP��ַ��Ϣ��htonl(INADDR_ANY)�ڷ����ָ����������IP��ַ��Ϣ; INADDR_ANY ���е�IP��ȥ��������֤�ͻ��˿�����������
		addrSrv.sin_family = AF_INET;//����ĵ�ַ��,IP����
		addrSrv.sin_port = htons(6000);//��������˿ں�
		if (SOCKET_ERROR == bind(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) {
			printf("bind error=%d\n", WSAGetLastError());
			return false;
		}
		//3 listen
		if (SOCKET_ERROR == listen(m_sockSrv, 1)) {//Զ�̼����һ��һ�ģ����Լ���1������
			printf("listen error=%d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	bool Accept() {
		//4 accept
		SOCKADDR_IN addrCli;
		int len = sizeof(SOCKADDR);
		m_sockcli = accept(m_sockSrv, (SOCKADDR*)&addrCli, &len);
		if (m_sockcli == INVALID_SOCKET)
			return false;
		return true;
		//closesocket(m_sockcli);
	}
	int Recv(char* buf) {
		if (m_sockcli == INVALID_SOCKET)return -1;//����ֻ�����������û�취���շ��͵�
		int ret = recv(m_sockcli, buf, 1024, 0);
		if (ret == SOCKET_ERROR) {
			printf("recv error=%d\n", WSAGetLastError());
			return -1;
		}
		//TODO:�����������
		return ret;
	}
	bool Send(const char* pData,int nSize) {
		if (m_sockcli == INVALID_SOCKET)return -1;
		return send(m_sockcli, pData, nSize, 0)>0;
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
};

//extern CServerSocket server;