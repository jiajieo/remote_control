#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//�Ը����ʵ�����г�ʼ��
CClientSocket::CHelper CClientSocket::m_helper;

std::string GetError(int a) {//a:WSAGetLastError() �����Ĳ���һ������д�궨��ġ�
	std::string Err;
	LPTSTR ErrnoText = NULL;
	//��ȡ������Ϣ
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, a, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&ErrnoText, 0, NULL);
	Err = (char*)ErrnoText;
	if (ErrnoText != NULL) {
		LocalFree(ErrnoText);//�ͷ��ڴ������
		ErrnoText = NULL;
	}
	return Err;
}