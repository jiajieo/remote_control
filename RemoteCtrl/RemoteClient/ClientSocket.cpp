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
	m_lock.lock();//上锁
	m_listSend.push_back(pack);//将发送的数据加入到发送队列里
	m_lock.unlock();
	if (m_sockCli == INVALID_SOCKET) {
		//	if (InitSocket() == false)return false;
		_beginthread(&threadSendPacket, 0, this);

	}
	//_beginthread(threadSendPacket, 0, this);
	m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPack));//保存发送数据的事件对象
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	WaitForSingleObject(pack.hEvent, INFINITE);//等待上一个命令发送完，事件对象变为有信号
	std::map<HANDLE, std::list<CPacket>&>::iterator it;
	it = m_mapAck.find(pack.hEvent);//在接收数据的队列里查找该事件对象，因为每次创建的事件对象的句柄都是独立的，它们具有不同的句柄和状态
	if (it != m_mapAck.end()) {
		//std::list<CPacket>::iterator i;//将与该事件对象同步的数据包队列遍历输出
		//for (i = it->second.begin(); i != it->second.end(); i++) {
		//	lstPack.push_back(*i);

		//}
		CloseHandle(pack.hEvent);
		m_mapAck.erase(it);

		return true;
	}
	return false;
}

void CClientSocket::threadFunc()//线程开始执行就不会退出，一直处理数据的发送接收，线程连同套接字网络都不能断开，每处理完一个命令由服务端断开分机，客户端不用再关闭了，防止两个功能被断开一个而影响下一个。如果被服务端断开，那客户端每次发送命令时再重新连接就好了
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
			InitSocket();//只要有数据要发送就连接
			TRACE("lstpack.size()=%d\n", m_listSend.size());
			CPacket& head = m_listSend.front();
			if (Send(head) == false) {
				TRACE("发送失败\r\n");
				continue;//失败就重新再连一次
			}
			//auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));//保存发送数据的事件对象
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

					//如果接收失败或者接收为0，就结束该命令事件
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
					it->second.push_back(pack);//将处理好的数据包也放入接收容器
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
