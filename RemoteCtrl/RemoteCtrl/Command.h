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
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>& lstPacket, CPacket& packet);//typedef作用是定义一个名为 CMDFUNC 的类型，是一个指向 CCommand 成员函数的指针类型
	std::map<int, CMDFUNC> m_mapFunction;//从命令号到功能的映射
	
	unsigned threadid;
	//CLockDialog dlg;//定义在全局，为了确保整个程序都可以访问该对话框
	
	//public:
	//	static int ExcuteCommand(int nCmd);
private:
	int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& packet);//查看磁盘分区
	int MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& packet);//查看指定目录下的文件
	int RunFile(std::list<CPacket>& lstPacket, CPacket& packet);//打开文件
	int DownloadFile(std::list<CPacket>& lstPacket, CPacket& packet);//下载文件
	int MouseEvent(std::list<CPacket>& lstPacket, CPacket& packet);//鼠标操作
	int SendScreen(std::list<CPacket>& lstPacket, CPacket& packet);//远程监控，发送屏幕截图给控制端
	int DelFile(std::list<CPacket>& lstPacket, CPacket& packet);//删除文件
	int LockMachine(std::list<CPacket>& lstPacket, CPacket& packet);//锁机
	int UnLockMachine(std::list<CPacket>& lstPacket, CPacket& packet);//解锁
	int ConnectTect(std::list<CPacket>& lstPacket, CPacket& packet);//测试连接
	static unsigned __stdcall threadLockDlg(void* arg);//锁机线程
	void Lock();
	
};

