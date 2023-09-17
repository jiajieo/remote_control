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
CServerSocket* pServer;

using namespace std;



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
			// TODO: 在此处为应用程序的行为编写代码。
			// 1.通过静态函数调用网络库初始化
			pServer = CServerSocket::getInstance();//类里面的静态函数不用声明对象可以直接调用。
			CCommand Cmd;
			if (pServer != NULL) {
				// 2.套接字的创建到监听
				if (pServer->InitSocket() == false) {
					MessageBox(NULL, _T("套接字创建监听失败！"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
					exit(0);//终止进程
				}
			}
			int count = 0;// 3.提供3次连接机会；服务端前面的初始化网络库和创建套接字不会变也不会关，只处理下面的被监控等待连接操作
			while (pServer != NULL) {
				if (pServer->Accept() == false) {
					if (count >= 3) {
						MessageBox(NULL, _T("服务端多次连接失败！结束程序"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
						exit(0);
					}
					MessageBox(NULL, _T("服务端连接失败，自动重试！"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
					count++;
				}
				//连接成功处理操作
				//接收命令
				int ret = pServer->Recv();

				if (ret > 0) {
					ret = Cmd.ExcuteCommand(pServer->Getpacket().sCmd);//执行命令
					if (ret != 0) {//命令执行成功会返回0
						TRACE("执行命令失败:sCmd=%d  ret=%d\n", pServer->Getpacket().sCmd, ret);
					}
					//远程控制采用短连接
					pServer->CloseClient();
				}
				//TODO:

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
