// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "Command.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的应用程序对象

CWinApp theApp;
using namespace std;
//CLockDialog dlg;


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
			// 1.通过静态函数调用网络库初始化
			CCommand Cmd;
			int ret=CServerSocket::getInstance()->Run(&CCommand::RunCCommand, &Cmd);//类里面的静态函数不用声明对象可以直接调用。
			switch (ret) {
			case -1:
				MessageBox(NULL, _T("套接字创建监听失败！"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
				exit(0);//终止进程
				break;
			case -2:
				MessageBox(NULL, _T("服务端多次连接失败！结束程序"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
				exit(0);
				break;
			}
			//dlg.Create(IDD_DIALOG_LOCK,NULL);//非模态对话框的创建 将非模态对话框的创建在主函数中完成，是确保非模态对话框的创建在程序启东时就完成，避免程序在运行过程中动态创建对话框可能带来的延迟和不确定性，同时在主函数中创建对话框可以更好地控制对话框的生命周期和与其他组件的交互。
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
