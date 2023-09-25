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
	CPacket(const BYTE* pData, size_t& nSize) {//方便解析数据，随便丢进来一个数据进行解析  BYTE:unsigned char BYTE 1字节
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

	int Size() const{//数据包的长度
		return nLength + 2 + 4;
	}

	const char* Data(std::string& strOut) const{//将整个数据包的数据转成字符串型，方便查看读取。
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
	//std::string strOut;//整个包的数据，方便查看
};
#pragma pack(pop)


std::string GetError(int a);//a:WSAGetLastError() 函数的参数一定不能写宏定义的。

#define BUFFER_SIZE 8192000//接收数据包的缓冲区大小
class CClientSocket
{
public:
	static CClientSocket* getInstance() {//静态函数没有this指针，无法直接访问成员变量；静态是全局的跟this指针是无关的，完全可以脱离this指针来调用这个静态方法
		if (m_instance == NULL) {
			m_instance = new CClientSocket();//new 伴随着delete，一定要清空，否则无法执行析构
		}
		return m_instance;
	}

	bool InitSocket() {//套接字创建及监听
		//1 创建套接字
		if (m_sockCli != INVALID_SOCKET) {
			closesocket(m_sockCli);
			m_sockCli = INVALID_SOCKET;
		}
		m_sockCli = socket(PF_INET, SOCK_STREAM, 0);//AF_INET和PF_INET混用没太大问题，但指定上建立socket指定协议应该用PF_INET，设置地址时用AF_INET；这里使用TCP而不是UDP，因为要求发送的数据是可信的
		if (INVALID_SOCKET == m_sockCli) {
			AfxMessageBox("socket套接字创建失败!");
			return false;
		}
		//2 connect
		SOCKADDR_IN addrCli;
		memset(&addrCli, 0, sizeof(addrCli));//定义的结构体要清空
		addrCli.sin_addr.S_un.S_addr = htonl(m_nIP);//用来保存客户端IP地址信息; inet_addr("")是将IP地址的字符串转换为 IP地址结构体的正确地址; htonl()从整型变量转换到IP地址结构体; ntohl()IP地址结构体转换成整形变量
		addrCli.sin_family = AF_INET;//传输的地址族,IP类型
		addrCli.sin_port = htons(m_port);//用来保存端口号
		if (addrCli.sin_addr.s_addr == INADDR_NONE) {//INADDR_NONE 指IP地址不合法或为空
			AfxMessageBox("指定的IP地址不存在！");
			return false;
		}
		if (SOCKET_ERROR == connect(m_sockCli, (sockaddr*)&addrCli, sizeof(SOCKADDR))) {
			TRACE("connect error=%d %s\n", WSAGetLastError(), GetError(WSAGetLastError()).c_str());
			AfxMessageBox("connect连接失败!");//在屏幕上显示一个消息框; 改成多字节字符
			return false;
		}
		return true;
	}
	int Recv() {
		if (m_sockCli == INVALID_SOCKET) {
			TRACE("客户端套接字错误\n");
			return -1;//如果套接字发生错误是没办法接收发送的
		}
		//char buffer[BUFFER_SIZE] = "";
		//char* buffer = new char[BUFFER_SIZE];
		char* buffer = m_buffer.data();//返回指定向量中第一个元素的指针，这是为了防止因泄露内存导致数据包丢失
		memset(buffer, 0, sizeof(buffer));
		size_t index = 0;//数据包接收定位
		//数据接收处理
		while (true) {
			size_t len = recv(m_sockCli, buffer + index, BUFFER_SIZE - index, 0);
			TRACE("recv datalen=%d\n", len);
			if (len == SOCKET_ERROR || len == 0) {
				TRACE("recv error=%d %s\n", WSAGetLastError(), GetError(WSAGetLastError()).c_str());
				return -1;
			}
			index += len;
			len = index;
			m_pack = CPacket((BYTE*)buffer, len);//解析数据
			if (len > 0) {//解析成功
				memmove(buffer, buffer + len, BUFFER_SIZE - len);//将buffer len后面的数据移到包头继续工作。memmove将缓冲区移到另一个缓冲区
				index -= len;
				//memcpy((void*)strData.c_str(), m_pack.strData.c_str(), m_pack.strData.length());
				//strData = m_pack.strData.c_str();//将收到的数据返回
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
	bool Send(const CPacket& pack) {//函数重载 加一个可以发送数据包的Send函数
		if (m_sockCli == INVALID_SOCKET)return false;
		std::string strOut;
		return send(m_sockCli, pack.Data(strOut), pack.Size(), 0) > 0;//将CPacket类转成const char*型(const char*)&pack
	}

	bool GetFilePath(std::string& strPath) {//获取控制端想要访问的路径
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

	CPacket& Getpacket() {//获取接受到的数据包
		return m_pack;
	}

	void CloseSocket() {
		closesocket(m_sockCli);
		m_sockCli = INVALID_SOCKET;
	}
	//刷新IP地址和端口号
	void UpdataAddress(const DWORD nIP,const int port) {
		m_nIP = nIP;
		m_port = port;
	}

private://这里包括它的复制，赋值构造函数都要写为私有的，不能让外部的进行构造
	//单例其实就不需要复制构造函数了。
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
	CClientSocket():m_nIP(INADDR_ANY),m_port(0) {//无效IP、无效端口
		m_sockCli = INVALID_SOCKET;
		m_buffer.resize(BUFFER_SIZE);//为该动态数组设定新的内存大小

		if (WSAServerInit() == FALSE) {
			MessageBox(NULL, _T("无法初始化网络套接字库"), _T("套接字初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);//关闭所有文件，终止正在执行的进程
		}
		
	}
	~CClientSocket() {
		closesocket(m_sockCli);
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
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	class CHelper {//因为CServerSocket类的析构没被调用到，所以在定义一个类来处理CServerSocket的析构问题
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};

private://成员变量是需要依赖这个类的实现的，是组合关系；但如果是指针就会从组合关系降为依赖关系
	//对IP和端口号的存放
	int m_port;
	DWORD m_nIP;

	std::vector<char> m_buffer;
	static CClientSocket* m_instance;//这里因为是类里的静态函数要访问的实例，所以要设为静态的
	static CHelper m_helper;
	SOCKET m_sockCli;//定义套接字;非静态变量在构造函数里初始化
	CPacket m_pack;//接收的数据包处理
};