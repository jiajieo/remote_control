// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

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

int MakeDriverInfo() {//创建一个磁盘分区
	std::string result;
	for (int i = 1; i <= 26; i++) {//查找磁盘分区
		if (_chdrive(i) == 0) {//更改当前的工作驱动器 1=>(代表)A盘 2=>B盘 ... 最多到26=>Z盘
			//存在的磁盘
			if (result.size() > 0) {
				result += ',';//将每个磁盘用","分隔开
			}
			result += 'A' + i - 1;//string型字符串可以直接通过+的方式将字符进行拼接
		}
	}
	//对数据进行打包
	CPacket pack(1, result.c_str(), result.size());
	Dump((BYTE*)pack.Data(), pack.Size());
	//拿到实例发送数据
	//CServerSocket::getInstance()->Send(pack);
	return 0;
}

int main()
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);//返回当前指定模块的句柄，nullptr表示返回创建调用进程.exe文件的句柄

	if (hModule != nullptr)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), SW_HIDE))//::GetCommandLine()检索当前进程的命令行字符串
		{
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC 初始化失败\n");
			nRetCode = 1;
		}
		else
		{
			//// TODO: 在此处为应用程序的行为编写代码。
			//// 1.通过静态函数调用网络库初始化
			//CServerSocket* pServer = CServerSocket::getInstance();//类里面的静态函数不用声明对象可以直接调用。
			//if (pServer != NULL) {
			//	// 2.套接字的创建到监听
			//	if (pServer->InitSocket() == false) {
			//		MessageBox(NULL, _T("套接字创建监听失败！"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
			//		exit(0);//终止进程
			//	}
			//}
			//int count = 0;// 3.提供3次连接机会；服务端前面的初始化网络库和创建套接字不会变也不会关，只处理下面的被监控等待连接操作
			//while (pServer != NULL) {
			//	if (pServer->Accept() == false) {
			//		if (count >= 3) {
			//			MessageBox(NULL, _T("服务端多次连接失败！结束程序"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
			//			exit(0);
			//		}
			//		MessageBox(NULL, _T("服务端连接失败，自动重试！"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
			//		count++;
			//	}
			//	//连接成功处理操作
			//	int ret = pServer->Recv();
			//	//TODO:
			//}

			// TODO:新功能 文件处理
			int nCmd = 0;//命令
			switch (nCmd) {
			case 1://查看磁盘分区
				MakeDriverInfo();
				break;
			case 2:

				break;
			case 3:

				break;
			case 4:

				break;
			default:

				break;
			}

		}
	}
	else
	{
		// TODO: 更改错误代码以符合需要
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}

	return nRetCode;
}
