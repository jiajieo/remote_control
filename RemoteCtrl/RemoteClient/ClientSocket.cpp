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
				TRACE("发送失败\r\n");
				continue;
			}
			auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));//保存发送数据的事件对象
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
				pr.first->second.push_back(pack);//将处理好的数据包也放入接收容器
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
