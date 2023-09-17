#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;

//一般this非静态函数是在构造函数里初始化；而静态成员不能再构造函数里初始化，因为它是所有类下面的对象和子类对象共用的，所以一定要用下面这种显示的方式进行初始化
CServerSocket* CServerSocket::m_instance = NULL;//对该类的实例进行初始化
CServerSocket::CHelper CServerSocket::m_helper;

//CServerSocket* pserver = CServerSocket::getInstance();//在main()函数之前就调用
