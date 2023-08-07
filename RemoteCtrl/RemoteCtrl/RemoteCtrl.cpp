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

#include <io.h>
#include <list>

typedef struct fileinfo {//结构体默认是public,类默认是private.
	fileinfo() {
		IsInvalid = FALSE;//默认是有效的
		IsDirectory = -1;
		IsHasNext = TRUE;//默认是有后续的
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//判断该目录是否无效
	BOOL IsDirectory;//判断是目录(文件夹)还是文件 0:文件 1:目录
	BOOL IsHasNext;//文件是否还有后续 1:是 0:否
	char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;

int MakeDirectoryInfo() {//查看指定目录下的文件
	std::string strPath;
	//std::list<FILEINFO> lstFileInfo;//用该结构体定义一个链表，方便文件处理
	if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
		OutputDebugString(_T("错误，当前命令不是获取文件路径的命令！\n"));
		return -1;
	}
	//数据获取成功，查看文件处理
	if (_chdir(strPath.c_str()) != 0) {//更改当前工作目录，进入指定目录，与_findfirst可配合使用
		fileinfo tempfile;
		tempfile.IsInvalid = TRUE;//目录无效
		tempfile.IsDirectory = TRUE;
		tempfile.IsHasNext = FALSE;
		memcpy(tempfile.szFileName, strPath.c_str(), strPath.size());
		//lstFileInfo.push_back(tempfile);
		CPacket pack(2, (char*)&tempfile, sizeof(tempfile));
		CServerSocket::getInstance()->Send(pack);
		OutputDebugString(_T("该目录不存在！\n"));
		return -2;
	}
	_finddata_t filedata;
	int hfind = 0;
	if ((hfind = _findfirst("*", &filedata)) == -1) {//在当前目录下查找文件或子目录，提供与参数filespec中指定的文件名匹配的第一个实例。_findfirst接收一个通配符字符串作为参数，可以用于查找文件或文件夹。
		OutputDebugString(_T("没有找到该文件！\n"));//输出调试模式一般在Debug模式下，不过如果不加控制条件，在release模式下也可以使用
		return -3;
	}
	do {//此时要将文件信息生成一个列表，想办法将这些文件正常提交到控制端，所以这里要再写一个结构体
		fileinfo tempfile;//临时文件
		tempfile.IsDirectory = (filedata.attrib&_A_SUBDIR)!=0;//filedata.attrib&_A_SUBDIR 判断他是否有子目录(文件夹)
		memcpy(tempfile.szFileName, filedata.name, filedata.size);
		//lstFileInfo.push_back(tempfile);
		CPacket pack(2, (char*)&tempfile, sizeof(tempfile));
		CServerSocket::getInstance()->Send(pack);
	} while (_findnext(hfind, &filedata));//查找下一个名称
	//发送信息到控制端，但是如果该文件夹有大量的文件，一次性发完效果不是很好，所以要一个一个发，但是从用户体验方面来说，后者与用户有交互，可以随时反馈，所以这里选后者，结构体加一个是否还有后续的判断变量
	fileinfo tempfile;
	tempfile.IsHasNext = FALSE;
	CPacket pack(2, (char*)&tempfile, sizeof(tempfile));
	CServerSocket::getInstance()->Send(pack);
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
			case 2://查看指定目录下的文件
				MakeDirectoryInfo();
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
