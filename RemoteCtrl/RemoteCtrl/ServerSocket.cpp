#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;

//һ��this�Ǿ�̬�������ڹ��캯�����ʼ��������̬��Ա�����ٹ��캯�����ʼ������Ϊ��������������Ķ������������õģ�����һ��Ҫ������������ʾ�ķ�ʽ���г�ʼ��
CServerSocket* CServerSocket::m_instance = NULL;//�Ը����ʵ�����г�ʼ��
CServerSocket::CHelper CServerSocket::m_helper;

//CServerSocket* pserver = CServerSocket::getInstance();//��main()����֮ǰ�͵���