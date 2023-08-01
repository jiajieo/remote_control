#pragma once
class CServerSocket
{
public:
	static CServerSocket* getInstance() {//静态函数没有this指针，无法直接访问成员变量；静态是全局的跟this指针是无关的，完全可以脱离this指针来调用这个静态方法
		if (m_instance == NULL) {
			m_instance = new CServerSocket();//new 伴随着delete，一定要清空，否则无法执行析构
		}
		return m_instance;
	}

	bool InitSocket() {//套接字创建及监听
		//1 创建套接字
		m_sockSrv = socket(PF_INET, SOCK_STREAM, 0);//AF_INET和PF_INET混用没太大问题，但指定上建立socket指定协议应该用PF_INET，设置地址时用AF_INET；这里使用TCP而不是UDP，因为要求发送的数据是可信的
		if (INVALID_SOCKET == m_sockSrv) {
			printf("sockSrv error=%d\n", WSAGetLastError());
			return false;
		}
		//2 bind
		SOCKADDR_IN addrSrv;
		memset(&addrSrv, 0, sizeof(addrSrv));//定义的结构体要清空
		addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//用来保存IP地址信息，htonl(INADDR_ANY)在服务端指本机的所有IP地址信息; INADDR_ANY 所有的IP都去监听，保证客户端可以连上来。
		addrSrv.sin_family = AF_INET;//传输的地址族,IP类型
		addrSrv.sin_port = htons(6000);//用来保存端口号
		if (SOCKET_ERROR == bind(m_sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR))) {
			printf("bind error=%d\n", WSAGetLastError());
			return false;
		}
		//3 listen
		if (SOCKET_ERROR == listen(m_sockSrv, 1)) {//远程监控是一对一的，所以监听1个就行
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
		if (m_sockcli == INVALID_SOCKET)return -1;//如果分机发生错误是没办法接收发送的
		int ret = recv(m_sockcli, buf, 1024, 0);
		if (ret == SOCKET_ERROR) {
			printf("recv error=%d\n", WSAGetLastError());
			return -1;
		}
		//TODO:处理接收数据
		return ret;
	}
	bool Send(const char* pData,int nSize) {
		if (m_sockcli == INVALID_SOCKET)return -1;
		return send(m_sockcli, pData, nSize, 0)>0;
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
};

//extern CServerSocket server;