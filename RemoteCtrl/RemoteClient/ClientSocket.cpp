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

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPack, bool isAutoClosed)
{
	m_lock.lock();//����
	m_listSend.push_back(pack);//�����͵����ݼ��뵽���Ͷ�����
	m_lock.unlock();
	if (m_sockCli == INVALID_SOCKET) {
		//	if (InitSocket() == false)return false;
		_beginthread(&threadSendPacket, 0, this);

	}
	//_beginthread(threadSendPacket, 0, this);
	m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPack));//���淢�����ݵ��¼�����
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	WaitForSingleObject(pack.hEvent, INFINITE);//�ȴ���һ��������꣬�¼������Ϊ���ź�
	std::map<HANDLE, std::list<CPacket>&>::iterator it;
	it = m_mapAck.find(pack.hEvent);//�ڽ������ݵĶ�������Ҹ��¼�������Ϊÿ�δ������¼�����ľ�����Ƕ����ģ����Ǿ��в�ͬ�ľ����״̬
	if (it != m_mapAck.end()) {
		//std::list<CPacket>::iterator i;//������¼�����ͬ�������ݰ����б������
		//for (i = it->second.begin(); i != it->second.end(); i++) {
		//	lstPack.push_back(*i);

		//}
		CloseHandle(pack.hEvent);
		m_mapAck.erase(it);

		return true;
	}
	return false;
}

void CClientSocket::threadFunc()//�߳̿�ʼִ�оͲ����˳���һֱ�������ݵķ��ͽ��գ��߳���ͬ�׽������綼���ܶϿ���ÿ������һ�������ɷ���˶Ͽ��ֻ����ͻ��˲����ٹر��ˣ���ֹ�������ܱ��Ͽ�һ����Ӱ����һ�������������˶Ͽ����ǿͻ���ÿ�η�������ʱ���������Ӿͺ���
{
	//WaitForSingleObject(m_hEvent, INFINITE);
	/*while (m_nIP == INADDR_ANY || m_port == 0)
		Sleep(1);*/

	std::vector<char> pBuf;
	pBuf.resize(BUFFER_SIZE);
	char* buf = pBuf.data();
	
	while (true) {
		/*if (m_sockCli == INVALID_SOCKET) {
			InitSocket();
		}*/
		if (m_listSend.size() > 0) {
			//Sleep(1);
			InitSocket();//ֻҪ������Ҫ���;�����
			TRACE("lstpack.size()=%d\n", m_listSend.size());
			CPacket& head = m_listSend.front();
			if (Send(head) == false) {
				TRACE("����ʧ��\r\n");
				continue;//ʧ�ܾ���������һ��
			}
			//auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));//���淢�����ݵ��¼�����
			std::map<HANDLE, std::list<CPacket>&>::iterator it;
			it = m_mapAck.find(head.hEvent);
			std::map<HANDLE, bool>::iterator it0;
			it0 = m_mapAutoClosed.find(head.hEvent);
			memset(buf, 0, sizeof(buf));
			int index = 0;
			do {
				size_t len = recv(m_sockCli, buf + index, BUFFER_SIZE - index, 0);
				if (len == SOCKET_ERROR || len == 0) {
					TRACE("recv error=%d(%s)\n", WSAGetLastError(), GetError(WSAGetLastError()));
					//CloseSocket();

					//�������ʧ�ܻ��߽���Ϊ0���ͽ����������¼�
					SetEvent(head.hEvent);
					break;
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
					m_lock.lock();
					it->second.push_back(pack);//������õ����ݰ�Ҳ�����������
					m_lock.unlock();
					if (it0->second == true)
						SetEvent(head.hEvent);
				}
				else {
					memset(buf, 0, sizeof(buf));
					index = 0;
				}

			} while (it0->second == false);
			m_lock.lock();
			m_listSend.pop_front();
			m_lock.unlock();

			if (it0 != m_mapAutoClosed.end()) {
				m_mapAutoClosed.erase(it0);
			}

			//break;
		}
	}
	//CloseSocket();
}
