#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//对该类的实例进行初始化
CClientSocket::CHelper CClientSocket::m_helper;