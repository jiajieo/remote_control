#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//对该类的实例进行初始化
CClientSocket::CHelper CClientSocket::m_helper;

std::string GetError(int a) {//a:WSAGetLastError() 函数的参数一定不能写宏定义的。
	std::string Err;
	LPTSTR ErrnoText = NULL;
	//获取错误消息
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, a, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&ErrnoText, 0, NULL);
	Err = (char*)ErrnoText;
	if (ErrnoText != NULL) {
		LocalFree(ErrnoText);//释放内存对象句柄
		ErrnoText = NULL;
	}
	return Err;
}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPack, bool isAutoClosed)
{
	m_listSend.push_back(pack);//将发送的数据加入到发送队列里
	if (m_sockCli == INVALID_SOCKET) {
		//	if (InitSocket() == false)return false;
		_beginthread(&threadSendPacket, 0, this);

	}
	//_beginthread(threadSendPacket, 0, this);
	m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(pack.hEvent, std::list<CPacket>()));//保存发送数据的事件对象
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	WaitForSingleObject(pack.hEvent, INFINITE);//等待上一个命令发送完，事件对象变为有信号
	std::map<HANDLE, std::list<CPacket>>::iterator it;
	it = m_mapAck.find(pack.hEvent);//在接收数据的队列里查找该事件对象，因为每次创建的事件对象的句柄都是独立的，它们具有不同的句柄和状态
	if (it != m_mapAck.end()) {
		std::list<CPacket>::iterator i;//将与该事件对象同步的数据包队列遍历输出
		for (i = it->second.begin(); i != it->second.end(); i++) {
			lstPack.push_back(*i);

		}
		m_mapAck.erase(it);
		CloseHandle(m_hEvent);
		return true;
		//return false;
	}
	
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
		while (true) {
			/*if (m_sockCli == INVALID_SOCKET) {
				InitSocket();
			}*/
			if (m_listSend.size() > 0) {
				//Sleep(1);
				InitSocket();
				TRACE("lstpack.size()=%d\n", m_listSend.size());
				CPacket& head = m_listSend.front();
				if (Send(head) == false) {
					TRACE("发送失败\r\n");
					continue;
				}
				//auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));//保存发送数据的事件对象
				std::map<HANDLE, std::list<CPacket>>::iterator it;
				it = m_mapAck.find(head.hEvent);
				std::map<HANDLE, bool>::iterator it0;
				it0 = m_mapAutoClosed.find(head.hEvent);
				do {
					size_t len = recv(m_sockCli, buf + index, BUFFER_SIZE - index, 0);
					if (len == SOCKET_ERROR || len == 0) {
						TRACE("recv error=%d(%s)\n", WSAGetLastError(), GetError(WSAGetLastError()));
						CloseSocket();
						SetEvent(head.hEvent);
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
						it->second.push_back(pack);//将处理好的数据包也放入接收容器
						if (it0->second == true)
							SetEvent(head.hEvent);
					}
					else {
						memset(buf, 0, sizeof(buf));
						index = 0;
					}
				} while (it0->second == false);
				m_listSend.pop_front();

				break;
			}
		}
		CloseSocket();
	}
