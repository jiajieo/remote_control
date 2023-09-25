#pragma once
#include <string>
#include <vector>
#include "Tool.h"

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
	CPacket(const BYTE* pData, size_t& nSize) {//����������ݣ���㶪����һ�����ݽ��н���  BYTE:unsigned char BYTE 1�ֽ�
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

	int Size() const{//���ݰ��ĳ���
		return nLength + 2 + 4;
	}

	const char* Data(std::string& strOut) const{//���������ݰ�������ת���ַ����ͣ�����鿴��ȡ��
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
	//std::string strOut;//�����������ݣ�����鿴
};
#pragma pack(pop)


std::string GetError(int a);//a:WSAGetLastError() �����Ĳ���һ������д�궨��ġ�

#define BUFFER_SIZE 8192000//�������ݰ��Ļ�������С
class CClientSocket
{
public:
	static CClientSocket* getInstance() {//��̬����û��thisָ�룬�޷�ֱ�ӷ��ʳ�Ա��������̬��ȫ�ֵĸ�thisָ�����޹صģ���ȫ��������thisָ�������������̬����
		if (m_instance == NULL) {
			m_instance = new CClientSocket();//new ������delete��һ��Ҫ��գ������޷�ִ������
		}
		return m_instance;
	}

	bool InitSocket() {//�׽��ִ���������
		//1 �����׽���
		if (m_sockCli != INVALID_SOCKET) {
			closesocket(m_sockCli);
			m_sockCli = INVALID_SOCKET;
		}
		m_sockCli = socket(PF_INET, SOCK_STREAM, 0);//AF_INET��PF_INET����û̫�����⣬��ָ���Ͻ���socketָ��Э��Ӧ����PF_INET�����õ�ַʱ��AF_INET������ʹ��TCP������UDP����ΪҪ���͵������ǿ��ŵ�
		if (INVALID_SOCKET == m_sockCli) {
			AfxMessageBox("socket�׽��ִ���ʧ��!");
			return false;
		}
		//2 connect
		SOCKADDR_IN addrCli;
		memset(&addrCli, 0, sizeof(addrCli));//����Ľṹ��Ҫ���
		addrCli.sin_addr.S_un.S_addr = htonl(m_nIP);//��������ͻ���IP��ַ��Ϣ; inet_addr("")�ǽ�IP��ַ���ַ���ת��Ϊ IP��ַ�ṹ�����ȷ��ַ; htonl()�����ͱ���ת����IP��ַ�ṹ��; ntohl()IP��ַ�ṹ��ת�������α���
		addrCli.sin_family = AF_INET;//����ĵ�ַ��,IP����
		addrCli.sin_port = htons(m_port);//��������˿ں�
		if (addrCli.sin_addr.s_addr == INADDR_NONE) {//INADDR_NONE ָIP��ַ���Ϸ���Ϊ��
			AfxMessageBox("ָ����IP��ַ�����ڣ�");
			return false;
		}
		if (SOCKET_ERROR == connect(m_sockCli, (sockaddr*)&addrCli, sizeof(SOCKADDR))) {
			TRACE("connect error=%d %s\n", WSAGetLastError(), GetError(WSAGetLastError()).c_str());
			AfxMessageBox("connect����ʧ��!");//����Ļ����ʾһ����Ϣ��; �ĳɶ��ֽ��ַ�
			return false;
		}
		return true;
	}
	int Recv() {
		if (m_sockCli == INVALID_SOCKET) {
			TRACE("�ͻ����׽��ִ���\n");
			return -1;//����׽��ַ���������û�취���շ��͵�
		}
		//char buffer[BUFFER_SIZE] = "";
		//char* buffer = new char[BUFFER_SIZE];
		char* buffer = m_buffer.data();//����ָ�������е�һ��Ԫ�ص�ָ�룬����Ϊ�˷�ֹ��й¶�ڴ浼�����ݰ���ʧ
		memset(buffer, 0, sizeof(buffer));
		size_t index = 0;//���ݰ����ն�λ
		//���ݽ��մ���
		while (true) {
			size_t len = recv(m_sockCli, buffer + index, BUFFER_SIZE - index, 0);
			TRACE("recv datalen=%d\n", len);
			if (len == SOCKET_ERROR || len == 0) {
				TRACE("recv error=%d %s\n", WSAGetLastError(), GetError(WSAGetLastError()).c_str());
				return -1;
			}
			index += len;
			len = index;
			m_pack = CPacket((BYTE*)buffer, len);//��������
			if (len > 0) {//�����ɹ�
				memmove(buffer, buffer + len, BUFFER_SIZE - len);//��buffer len����������Ƶ���ͷ����������memmove���������Ƶ���һ��������
				index -= len;
				//memcpy((void*)strData.c_str(), m_pack.strData.c_str(), m_pack.strData.length());
				//strData = m_pack.strData.c_str();//���յ������ݷ���
				return m_pack.sCmd;
			}
			else {
				memset(buffer, 0, BUFFER_SIZE);
				index = 0;
			}
		}
		return -1;
	}
	bool Send(const char* pData, int nSize) {
		if (m_sockCli == INVALID_SOCKET)return false;
		return send(m_sockCli, pData, nSize, 0) > 0;
	}
	bool Send(const CPacket& pack) {//�������� ��һ�����Է������ݰ���Send����
		if (m_sockCli == INVALID_SOCKET)return false;
		std::string strOut;
		return send(m_sockCli, pack.Data(strOut), pack.Size(), 0) > 0;//��CPacket��ת��const char*��(const char*)&pack
	}

	bool GetFilePath(std::string& strPath) {//��ȡ���ƶ���Ҫ���ʵ�·��
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

	CPacket& Getpacket() {//��ȡ���ܵ������ݰ�
		return m_pack;
	}

	void CloseSocket() {
		closesocket(m_sockCli);
		m_sockCli = INVALID_SOCKET;
	}
	//ˢ��IP��ַ�Ͷ˿ں�
	void UpdataAddress(const DWORD nIP,const int port) {
		m_nIP = nIP;
		m_port = port;
	}

private://����������ĸ��ƣ���ֵ���캯����ҪдΪ˽�еģ��������ⲿ�Ľ��й���
	//������ʵ�Ͳ���Ҫ���ƹ��캯���ˡ�
	CClientSocket& operator = (const CClientSocket& ss) {
		if (this != &ss) {
			m_nIP = ss.m_nIP;
			m_port = ss.m_port;
			memcpy(&m_pack, &ss.m_pack, sizeof(CPacket));
			m_sockCli = ss.m_sockCli;
		}
		else {
			return *this;
		}
	}
	CClientSocket(const CClientSocket& ss) {
		m_nIP = ss.m_nIP;
		m_port = ss.m_port;
		memcpy(&m_pack, &ss.m_pack, sizeof(CPacket));
		m_sockCli = ss.m_sockCli;
	}
	CClientSocket():m_nIP(INADDR_ANY),m_port(0) {//��ЧIP����Ч�˿�
		m_sockCli = INVALID_SOCKET;
		m_buffer.resize(BUFFER_SIZE);//Ϊ�ö�̬�����趨�µ��ڴ��С

		if (WSAServerInit() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ�������׽��ֿ�"), _T("�׽��ֳ�ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);//�ر������ļ�����ֹ����ִ�еĽ���
		}
		
	}
	~CClientSocket() {
		closesocket(m_sockCli);
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
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	class CHelper {//��ΪCServerSocket�������û�����õ��������ڶ���һ����������CServerSocket����������
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};

private://��Ա��������Ҫ����������ʵ�ֵģ�����Ϲ�ϵ���������ָ��ͻ����Ϲ�ϵ��Ϊ������ϵ
	//��IP�Ͷ˿ںŵĴ��
	int m_port;
	DWORD m_nIP;

	std::vector<char> m_buffer;
	static CClientSocket* m_instance;//������Ϊ������ľ�̬����Ҫ���ʵ�ʵ��������Ҫ��Ϊ��̬��
	static CHelper m_helper;
	SOCKET m_sockCli;//�����׽���;�Ǿ�̬�����ڹ��캯�����ʼ��
	CPacket m_pack;//���յ����ݰ�����
};