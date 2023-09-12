#pragma once
void Dump(BYTE* pData, size_t nSize);

typedef struct fileinfo {//�ṹ��Ĭ����public,��Ĭ����private.
	fileinfo() {
		IsInvalid = FALSE;//Ĭ������Ч��
		IsDirectory = -1;
		IsHasNext = TRUE;//Ĭ�����к�����
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//�жϸ�Ŀ¼�Ƿ���Ч
	BOOL IsDirectory;//�ж���Ŀ¼(�ļ���)�����ļ� 0:�ļ� 1:Ŀ¼
	BOOL IsHasNext;//�ļ��Ƿ��к��� 1:�� 0:��
	char szFileName[256];//�ļ���
}FILEINFO, * PFILEINFO;

#pragma pack(push)
#pragma pack(1)
class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {

	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	//�������
	CPacket(WORD nCmd, const char* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;//+�����У���ǰ�����
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((char*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t i = 0; i < strData.size(); i++) {
			sSum += ((BYTE)strData[i] & 0xFF);
		}
	}
	CPacket(const BYTE* pData, size_t nSize) {//����������ݣ���㶪����һ�����ݽ��н���  BYTE:unsigned char BYTE 1�ֽ�
		size_t i = 0;//�ֽڳ��ȸ���
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {//�ҵ�һ����ͷ
				sHead = *(WORD*)(pData + i);
				i += 2;//������ͷ
				break;
			}
		}
		if ((i + 4 + 2 + 2) > nSize) {//δ�ҵ���ͷ���ͷ��������;i+4+2+2�ǳ�ȥ�����ȡ������У����ж��Ƿ������ݣ����û��ֱ���˳�����
			nSize = 0;//��nSize��0
			return;
		}
		nLength = *(DWORD*)(pData + i);//��ȡ������
		i += 4;
		if ((nLength + i) > nSize) {//����û��ȫ��ֻ�յ�һ�룬˵��������̫С�����ݰ�û�����������˳���������Ϊ�������Ǵ����ʼ����У�������������ʱ��ʼ�жϡ�
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);//��ȡ����
		i += 2;
		//��ȡ������
		if (nLength > 4) {//������>4�Ż��а����ݵ�λ��
			strData.resize(nLength - 2 - 2);//Ϊ�ַ���ָ���µĴ�С�����ð����ݵĴ�С
			memcpy((void*)strData.c_str(), (pData + i), (nLength - 2 - 2));//c_str()���ص�ǰ�ַ��������ַ���ַ��ָ���Կ��ַ���ֹ�����飻data()��c_str()���ƣ������ص����鲻�Կ��ַ���ֹ
			i += (nLength - 4);
		}
		sSum = *(WORD*)(pData + i);//��ȡ��У��
		i += 2;
		//У��һ��
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {//.size()�����ַ�����Ԫ�صĵ�ǰ��Ŀ
			sum += ((BYTE)strData[j] & 0xFF);//���ֶ����Ʋ����һ����
		}
		if (sum == sSum) {//�����ɹ�
			nSize = i;//�����������ݰ��ĳ��ȣ�����ʹ��i���������ݰ����ȣ�����Ϊǰ�滹�з�����
			return;
		}
		nSize = 0;//����ʧ��
	}
	~CPacket() {}

	CPacket operator=(const CPacket& pack) {//���������
		if (this != &pack) {//thisָ��������ı���
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size() {//���ݰ��ĳ���
		return nLength + 2 + 4;
	}

	const char* Data() {//���������ݰ�������ת���ַ����ͣ�����鿴��ȡ��
		strOut.resize(Size());//��strOut�ַ�����Сָ��Ϊ�������ݰ��ĳ���
		BYTE* pData = (BYTE*)strOut.c_str();//����һ��BYTE�������͵�ָ��ָ���������������ַ���
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), nLength - 4); pData += strData.size();
		*(WORD*)pData = sSum; pData += 2;
		return strOut.c_str();
	}

public://���������ⲿ��Ҫ���õ��ģ�����������public
	WORD sHead;//��ͷFE FF  unsigned short 2�ֽ�
	DWORD nLength;//�����ȣ����ݵ��ֽڳ���(�����ʼ������У�����)  unsigned long 4�ֽ�
	WORD sCmd;//����
	std::string strData;//������
	WORD sSum;//��У�� �������ݽ��м����
	std::string strOut;//�����������ݣ�����鿴0001000000000001
};
#pragma pack(pop)

typedef struct mouseev{
	mouseev(){//��ʼ��
		nAction = 0;
		nButton = 0;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//����������������:���(������˫��)���ƶ�
	WORD nButton;//������Ҽ����м�
	POINT ptXY;//����
}MOUSEEV,*PMOUSEEV;

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

	bool InitSocket() {//�׽��ִ���������
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
		addrSrv.sin_port = htons(6000);//��������˿ں�
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
		if ((m_pack.sCmd == 2) || (m_pack.sCmd == 3)|| (m_pack.sCmd == 4)||(m_pack.sCmd==7)) {//��������2��3�����Ի�ȡҪ���ʵ�·��
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
		closesocket(m_sockcli);
		m_sockcli = INVALID_SOCKET;
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
};

//extern CServerSocket server;