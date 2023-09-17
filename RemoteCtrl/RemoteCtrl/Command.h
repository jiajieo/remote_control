#pragma once
#include <map>
#include "ServerSocket.h"
#include "LockDialog.h"

class CCommand
{
public:
	CCommand();
	~CCommand() {}
	int ExcuteCommand(int nCmd);
	
protected:
	typedef int(CCommand::* CMDFUNC)();//typedef�����Ƕ���һ����Ϊ CMDFUNC �����ͣ���һ��ָ�� CCommand ��Ա������ָ������
	std::map<int, CMDFUNC> m_mapFunction;//������ŵ����ܵ�ӳ��
	
	unsigned threadid;
	CServerSocket* m_pServer;
	//CLockDialog dlg;//������ȫ�֣�Ϊ��ȷ���������򶼿��Է��ʸöԻ���
	
	//public:
	//	static int ExcuteCommand(int nCmd);
private:
	int MakeDriverInfo();//�鿴���̷���
	int MakeDirectoryInfo();//�鿴ָ��Ŀ¼�µ��ļ�
	int RunFile();//���ļ�
	int DownloadFile();//�����ļ�
	int MouseEvent();//������
	int SendScreen();//Զ�̼�أ�������Ļ��ͼ�����ƶ�
	int DelFile();//ɾ���ļ�
	int LockMachine();//����
	int UnLockMachine();//����
	int ConnectTect();//��������
	static unsigned __stdcall threadLockDlg(void* arg);//�����߳�
	void Lock();
	
};

