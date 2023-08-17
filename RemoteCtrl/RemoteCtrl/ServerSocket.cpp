#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;

//一般this非静态函数是在构造函数里初始化；而静态成员不能再构造函数里初始化，因为它是所有类下面的对象和子类对象共用的，所以一定要用下面这种显示的方式进行初始化
CServerSocket* CServerSocket::m_instance = NULL;//对该类的实例进行初始化
CServerSocket::CHelper CServerSocket::m_helper;

//CServerSocket* pserver = CServerSocket::getInstance();//在main()函数之前就调用

void Dump(BYTE* pData, size_t nSize) {//查看输出打包好的数据
	std::string strOut;
	char buf[10] = "";
	for (size_t i = 0; i < nSize; i++) {
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);//%02X:X表示以16进制输出，02表示以两位输出
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());//将字符串发送到调试器进行显示
	//结果发现数据不对：FFFECCCC090000000100CCCCD8EF41，然后首先将CPacket类的字节进行对齐操作
	//TRACE("%s\r\n", strOut.c_str());
}