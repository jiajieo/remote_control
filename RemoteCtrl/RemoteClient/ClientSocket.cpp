#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//�Ը����ʵ�����г�ʼ��
CClientSocket::CHelper CClientSocket::m_helper;

std::string GetError(int a) {//a:WSAGetLastError() �����Ĳ���һ������д�궨��ġ�
	std::string Err;
	LPTSTR ErrnoText = NULL;
	//��ȡ������Ϣ
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, a, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&ErrnoText, 0, NULL);
	Err = (char*)ErrnoText;
	if (ErrnoText != NULL) {
		LocalFree(ErrnoText);//�ͷ��ڴ������
		ErrnoText = NULL;
	}
	return Err;
}

void CClientSocket::threadFunc()
{
	//WaitForSingleObject(m_hEvent, INFINITE);
	/*while (m_nIP == INADDR_ANY || m_port == 0)
		Sleep(1);*/

	std::vector<char> pBuf;
	pBuf.resize(BUFFER_SIZE);
	char* buf = pBuf.data();
	memset(buf, 0, sizeof(buf));
	int index = 0;
	while (m_sockCli != INVALID_SOCKET) {
		if (m_listSend.size() > 0) {
			CPacket& head = m_listSend.front();
			if (Send(head) == false) {
				TRACE("����ʧ��\r\n");
				continue;
			}
			auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));//���淢�����ݵ��¼�����
			size_t len = recv(m_sockCli, buf + index, BUFFER_SIZE - index, 0);
			if (len == SOCKET_ERROR || len == 0) {
				TRACE("recv error=%d(%s)\n", WSAGetLastError(), GetError(WSAGetLastError()));
				CloseSocket();
			}
			len += index;
			index = len;
			CPacket pack((BYTE*)buf, len);
			//std::list<CPacket>lstRecv;
			if (len > 0) {
				memmove(buf, buf + len, index - len);
				index = index - len;
				/*m_listPack.push_back(pack);*/
				//pack.hEvent = head.hEvent;
				pr.first->second.push_back(pack);//������õ����ݰ�Ҳ�����������
				SetEvent(head.hEvent);
			}
			else {
				memset(buf, 0, sizeof(buf));
				index = 0;
			}
			m_listSend.pop_front();
			break;
		}
	}

	CloseSocket();
}
