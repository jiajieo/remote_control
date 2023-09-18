#pragma once
#include <list>
#include "CPacket.h"
#include "Tool.h"

typedef void(*SOCKET_CALLBACK)(void* arg, int cmd, std::list<CPacket>& lstPacket,CPacket&);//定义一个函数指针类型

#define BUFFER_SIZE 409600//接收数据包的缓冲区大小
class CServerSocket
{
public:
	static CServerSocket* getInstance() {//静态函数没有this指针，无法直接访问成员变量；静态是全局的跟this指针是无关的，完全可以脱离this指针来调用这个静态方法
		if (m_instance == NULL) {
			m_instance = new CServerSocket();//new 伴随着delete，一定要清空，否则无法执行析构
		}
		return m_instance;
	}

	int Run(SOCKET_CALLBACK callback, void* arg,short port=6000) {//arg是传递CCommand类对象
		std::list<CPacket> lstPacket;//定义一个容器用来存储打包好的数据。
		// 2.套接字的创建到监听
		if (InitSocket(port) == false)return -1;
		int count = 0;// 3.提供3次连接机会；服务端前面的初始化网络库和创建套接字不会变也不会关，只处理下面的被监控等待连接操作
		while (true) {
			if (Accept() == false) {
				if (count >= 2) {
					return -2;
				}
				count++;
			}
			//连接成功处理操作
			//接收命令
			int ret = Recv();
			callback(arg, ret,lstPacket,m_pack);
			while (lstPacket.size() > 0) {
				Sleep(1);
				Send(lstPacket.front());
				lstPacket.pop_front();
			}
			//远程控制采用短连接
			CloseClient();
		}
		/*m_callback = callback;
		m_arg = arg;*/
	}

protected:
	bool InitSocket(short port) {//套接字创建及监听
		//1 创建套接字
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);//AF_INET和PF_INET混用没太大问题，但指定上建立socket指定协议应该用PF_INET，设置地址时用AF_INET；这里使用TCP而不是UDP，因为要求发送的数据是可信的
		if (INVALID_SOCKET == m_sockSrv) {
			TRACE("sockSrv error=%d\n", WSAGetLastError());
			return false;
		}
		//2 bind
		SOCKADDR_IN addrSrv;
		memset(&addrSrv, 0, sizeof(addrSrv));//定义的结构体要清空
		addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//用来保存IP地址信息，htonl(INADDR_ANY)在服务端指本机的所有IP地址信息; INADDR_ANY 所有的IP都去监听，保证客户端可以连上来。
		addrSrv.sin_family = AF_INET;//传输的地址族,IP类型IPv4
		addrSrv.sin_port = htons(port);//用来保存端口号
		if (SOCKET_ERROR == bind(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) {
			TRACE("bind error=%d\n", WSAGetLastError());
			return false;
		}
		//3 listen
		if (SOCKET_ERROR == listen(m_sockSrv, 1)) {//远程监控是一对一的，所以监听1个就行
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
		if (m_sockcli == INVALID_SOCKET)return -1;//如果分机发生错误是没办法接收发送的
		char buffer[BUFFER_SIZE] = "";
		//char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));
		size_t index = 0;//数据包接收定位
		//数据接收处理
		while (true) {
			size_t len = recv(m_sockcli, buffer + index, BUFFER_SIZE - index, 0);
			if (len == SOCKET_ERROR) {
				TRACE("recv error=%d\n", WSAGetLastError());
				return -1;
			}
			index += len;
			len = index;
			m_pack = CPacket((BYTE*)buffer, len);//解析数据
			//Dump((BYTE*)m_pack.Data(), m_pack.Size());
			if (len > 0) {//解析成功
				memmove(buffer, buffer + len, BUFFER_SIZE - len);//将buffer len后面的数据移到包头继续工作。memmove将缓冲区移到另一个缓冲区
				index -= len;
				return m_pack.sCmd;//将收到的命令返回
			}
			else {
				memset(buffer, 0, BUFFER_SIZE);
				index = 0;
			}
		}
		return -1;
	}

	CPacket Getpacket() {//获取数据包用于验证
		return m_pack;
	}

	bool Send(const char* pData, int nSize) {
		if (m_sockcli == INVALID_SOCKET)return false;
		return send(m_sockcli, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {//函数重载 加一个可以发送数据包的Send函数
		if (m_sockcli == INVALID_SOCKET)return false;
		TRACE("serv datalen=%d\n", pack.Size());
		return send(m_sockcli, pack.Data(), pack.Size(), 0) > 0;//将CPacket类转成const char*型(const char*)&pack
	}

	bool GetFilePath(std::string& strPath) {//控制端想要获取文件路径的权限
		if ((m_pack.sCmd == 2) || (m_pack.sCmd == 3) || (m_pack.sCmd == 4) || (m_pack.sCmd == 7)) {//控制命令2和3都可以获取要访问的路径
			strPath = m_pack.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEV& mouse) {//这里选择用应用来指向传进来参数的地址
		if (m_pack.sCmd == 5) {//定义一个结构体来获取鼠标的一些属性，比如：左键，右键，单击，双击，移动等
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

private://这里包括它的复制，赋值构造函数都要写为私有的，不能让外部的进行构造
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
			MessageBox(NULL, _T("无法初始化网络套接字库"), _T("套接字初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);//关闭所有文件，终止正在执行的进程
		}
	}
	~CServerSocket() {
		closesocket(m_sockSrv);
		WSACleanup();//在判断有没有执行析构函数时要把等待接收发送部分注释掉
	}

	BOOL WSAServerInit() {
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(1, 1);//为了尽可能在更多机器上运行，这里采用低一点的1.1版本，
		// 初始化套接字库
		int err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0)
		{
			return FALSE;
		}
		if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)//这里 MAKEWORD(BYTE bLow,BYTE bHigh) 设的版本号有关(低位字节和高位字节)
		{
			WSACleanup();
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance() {//处理m_instance要搞成静态的
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	class CHelper {//因为CServerSocket类的析构没被调用到，所以在定义一个类来处理CServerSocket的析构问题
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};

private:
	static CServerSocket* m_instance;//这里因为是类里的静态函数要访问的实例，所以要设为静态的
	static CHelper m_helper;
	SOCKET m_sockSrv;//定义套接字;非静态变量在构造函数里初始化
	SOCKET m_sockcli;
	CPacket m_pack;//接收的数据包处理

	SOCKET_CALLBACK m_callback;
	void* m_arg;
};

//extern CServerSocket server;