#pragma once
void Dump(BYTE* pData, size_t nSize);

typedef struct fileinfo {//结构体默认是public,类默认是private.
	fileinfo() {
		IsInvalid = FALSE;//默认是有效的
		IsDirectory = -1;
		IsHasNext = TRUE;//默认是有后续的
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//判断该目录是否无效
	BOOL IsDirectory;//判断是目录(文件夹)还是文件 0:文件 1:目录
	BOOL IsHasNext;//文件是否还有后续 1:是 0:否
	char szFileName[256];//文件名
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
	//打包数据
	CPacket(WORD nCmd, const char* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;//+命令和校验是包长度
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
	CPacket(const BYTE* pData, size_t nSize) {//方便解析数据，随便丢进来一个数据进行解析  BYTE:unsigned char BYTE 1字节
		size_t i = 0;//字节长度跟踪
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {//找到一个包头
				sHead = *(WORD*)(pData + i);
				i += 2;//跳过包头
				break;
			}
		}
		if ((i + 4 + 2 + 2) > nSize) {//未找到包头或包头后无数据;i+4+2+2是除去包长度、命令、和校验后判断是否还有数据，如果没有直接退出函数
			nSize = 0;//将nSize清0
			return;
		}
		nLength = *(DWORD*)(pData + i);//获取包长度
		i += 4;
		if ((nLength + i) > nSize) {//数据没收全，只收到一半，说明缓冲区太小，数据包没接收完整，退出函数；因为包长度是从命令开始，到校验结束，所以这时开始判断。
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);//获取命令
		i += 2;
		//获取包数据
		if (nLength > 4) {//包长度>4才会有包数据的位置
			strData.resize(nLength - 2 - 2);//为字符串指定新的大小；设置包数据的大小
			memcpy((void*)strData.c_str(), (pData + i), (nLength - 2 - 2));//c_str()返回当前字符串的首字符地址，指向以空字符终止的数组；data()与c_str()类似，但返回的数组不以空字符终止
			i += (nLength - 4);
		}
		sSum = *(WORD*)(pData + i);//获取和校验
		i += 2;
		//校验一下
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {//.size()返回字符串中元素的当前数目
			sum += ((BYTE)strData[j] & 0xFF);//保持二进制补码的一致性
		}
		if (sum == sSum) {//解析成功
			nSize = i;//等于整个数据包的长度，这里使用i而不是数据包长度，是因为前面还有废数据
			return;
		}
		nSize = 0;//解析失败
	}
	~CPacket() {}

	CPacket operator=(const CPacket& pack) {//运算符重载
		if (this != &pack) {//this指向类自身的变量
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size() {//数据包的长度
		return nLength + 2 + 4;
	}

	const char* Data() {//将整个数据包的数据转成字符串型，方便查看读取。
		strOut.resize(Size());//将strOut字符串大小指定为整个数据包的长度
		BYTE* pData = (BYTE*)strOut.c_str();//定义一个BYTE数据类型的指针指向整个包的数据字符串
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), nLength - 4); pData += strData.size();
		*(WORD*)pData = sSum; pData += 2;
		return strOut.c_str();
	}

public://包数据是外部需要调用到的，所以这里用public
	WORD sHead;//包头FE FF  unsigned short 2字节
	DWORD nLength;//包长度，数据的字节长度(从命令开始，到和校验结束)  unsigned long 4字节
	WORD sCmd;//命令
	std::string strData;//包数据
	WORD sSum;//和校验 将包数据进行加求和
	std::string strOut;//整个包的数据，方便查看0001000000000001
};
#pragma pack(pop)

typedef struct mouseev{
	mouseev(){//初始化
		nAction = 0;
		nButton = 0;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//首先是描述动作的:点击(单击、双击)、移动
	WORD nButton;//左键、右键、中键
	POINT ptXY;//坐标
}MOUSEEV,*PMOUSEEV;

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

	bool InitSocket() {//套接字创建及监听
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
		addrSrv.sin_port = htons(6000);//用来保存端口号
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
		if ((m_pack.sCmd == 2) || (m_pack.sCmd == 3)|| (m_pack.sCmd == 4)||(m_pack.sCmd==7)) {//控制命令2和3都可以获取要访问的路径
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
		closesocket(m_sockcli);
		m_sockcli = INVALID_SOCKET;
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
};

//extern CServerSocket server;