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
	typedef int(CCommand::* CMDFUNC)();//typedef作用是定义一个名为 CMDFUNC 的类型，是一个指向 CCommand 成员函数的指针类型
	std::map<int, CMDFUNC> m_mapFunction;//从命令号到功能的映射
	
	unsigned threadid;
	CServerSocket* m_pServer;
	//CLockDialog dlg;//定义在全局，为了确保整个程序都可以访问该对话框
	
	//public:
	//	static int ExcuteCommand(int nCmd);
private:
	int MakeDriverInfo();//查看磁盘分区
	int MakeDirectoryInfo();//查看指定目录下的文件
	int RunFile();//打开文件
	int DownloadFile();//下载文件
	int MouseEvent();//鼠标操作
	int SendScreen();//远程监控，发送屏幕截图给控制端
	int DelFile();//删除文件
	int LockMachine();//锁机
	int UnLockMachine();//解锁
	int ConnectTect();//测试连接
	static unsigned __stdcall threadLockDlg(void* arg);//锁机线程
	void Lock();
	
};

