#pragma once
#include <map>
#include <list>
#include "CPacket.h"
#include "LockDialog.h"


class CCommand
{
public:
	CCommand();
	~CCommand() {}
	int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& packet);
	static void RunCCommand(void* arg,int status, std::list<CPacket>& lstPacket,CPacket& packet);
	
protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>& lstPacket, CPacket& packet);//typedef�����Ƕ���һ����Ϊ CMDFUNC �����ͣ���һ��ָ�� CCommand ��Ա������ָ������
	std::map<int, CMDFUNC> m_mapFunction;//������ŵ����ܵ�ӳ��
	
	unsigned threadid;
	//CLockDialog dlg;//������ȫ�֣�Ϊ��ȷ���������򶼿��Է��ʸöԻ���
	
	//public:
	//	static int ExcuteCommand(int nCmd);
private:
	int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& packet);//�鿴���̷���
	int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& packet);//�鿴ָ��Ŀ¼�µ��ļ�
	int RunFile(std::list<CPacket>& lstPacket, CPacket& packet);//���ļ�
	int DownloadFile(std::list<CPacket>& lstPacket, CPacket& packet);//�����ļ�
	int MouseEvent(std::list<CPacket>& lstPacket, CPacket& packet);//������
	int SendScreen(std::list<CPacket>& lstPacket, CPacket& packet);//Զ�̼�أ�������Ļ��ͼ�����ƶ�
	int DelFile(std::list<CPacket>& lstPacket, CPacket& packet);//ɾ���ļ�
	int LockMachine(std::list<CPacket>& lstPacket, CPacket& packet);//����
	int UnLockMachine(std::list<CPacket>& lstPacket, CPacket& packet);//����
	int ConnectTect(std::list<CPacket>& lstPacket, CPacket& packet);//��������
	static unsigned __stdcall threadLockDlg(void* arg);//�����߳�
	void Lock();
	
};

