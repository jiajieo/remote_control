#include "pch.h"
#include "Command.h"
#include <direct.h>
#include "Tool.h"
#include <io.h>
#include <atlimage.h>
#include "Resource.h"

CLockDialog dlg;//定义在全局，为了确保整个程序都可以访问该对话框

CCommand::CCommand()
{
	struct {
		int nCmd;
		CMDFUNC func;
	}data[] = {//后面data是变量，用来初始化
		{1,&CCommand::MakeDriverInfo},//查看磁盘分区
		{2,&CCommand::MakeDirectoryInfo},//查看指定目录下的文件
		{3,&CCommand::RunFile},//打开文件
		{4,&CCommand::DownloadFile},//下载文件
		{5,&CCommand::MouseEvent},//鼠标操作
		{6,&CCommand::SendScreen},//远程监控，发送屏幕截图给控制端
		{7,&CCommand::DelFile},//删除文件
		{8,&CCommand::LockMachine},//锁机
		{9,&CCommand::UnLockMachine},//解锁
		{1981,&CCommand::ConnectTect},//测试连接
		{-1,NULL}
	};
	for (int i = 0; data[i].nCmd != -1; i++) {
		m_mapFunction.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func));//将初始化的键值对都插入到map容器中
	}
}

int CCommand::ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& packet)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);//定义一个迭代器能够取出map容器里面的数据
	if (it == m_mapFunction.end())//m_mapFunction.end()返回指向容器末尾的迭代器
	{//没找到该命令
		return -1;
	}
	//找到该命令就返回该元素的成员函数指针对象
	return (this->*(it->second))(lstPacket, packet);//this->*是取了我的一个成员
}

void CCommand::RunCCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& packet)//arg返回该类的对象，status是接收到的命令
{
	CCommand* thiz = (CCommand*)arg;
	if (status > 0) {
		if (thiz->ExcuteCommand(status, lstPacket, packet) != 0) {//命令执行成功会返回0
			TRACE("执行命令失败:sCmd=%d\n", status);
		}
	}
	else {
		MessageBox(NULL, _T("服务端连接失败，自动重试！"), _T("错误"), MB_OK | MB_ICONERROR);//NULL表示悬空弹窗窗口
	}
}


//int CCommand::ExcuteCommand(int nCmd)
//{
//	int ret;
//	switch (nCmd) {
//	case 1://查看磁盘分区
//		ret = MakeDriverInfo();
//		break;
//	case 2://查看指定目录下的文件
//		ret = MakeDirectoryInfo();
//		break;
//	case 3://打开文件
//		ret = RunFile();
//		break;
//	case 4://下载文件
//		ret = DownloadFile();
//		break;
//	case 5://鼠标操作
//		ret = MouseEvent();
//		break;
//	case 6://远程监控，发送屏幕截图给控制端
//		ret = SendScreen();
//		break;
//	case 7://删除文件
//		ret = DelFile();
//		break;
//	case 8://锁机
//		ret = LockMachine();
//		Sleep(50);//等待当前线程的执行
//		//LockMachine();
//		break;
//	case 9://解锁
//		ret = UnLockMachine();
//		while (dlg.m_hWnd != NULL) {
//			Sleep(100);//等待线程执行结束；不能直接退出，因为线程还未析构
//		}
//		break;
//	case 1981://测试连接
//		ConnectTect();
//		break;
//	}
//	//Sleep(5000);//等待当前锁机线程执行5s
//	//UnLockMachine();//解锁
//	return 0;
//}

int CCommand::MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& packet)//创建一个磁盘分区
{
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
	lstPacket.push_back(CPacket(1, result.c_str(), result.size()));//从尾部插入到链表容器
	//CTool::Dump((BYTE*)pack.Data(), pack.Size());
	//拿到实例发送数据
	//m_pServer->Send(pack);
	return 0;
}

int CCommand::MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& packet)//查看指定目录下的文件
{
	std::string strPath = packet.strData;
	//std::list<FILEINFO> lstFileInfo;//用该结构体定义一个链表，方便文件处理
	/*if (m_pServer->GetFilePath(strPath) == false) {
		OutputDebugString(_T("错误，当前命令不是获取文件路径的命令！\n"));
		return -1;
	}*/
	//数据获取成功，查看文件处理
	if (_chdir(strPath.c_str()) != 0) {//更改当前工作目录，进入指定目录，与_findfirst可配合使用
		//目录不存在的情况
		fileinfo tempfile;
		tempfile.IsInvalid = TRUE;//目录无效
		tempfile.IsHasNext = FALSE;
		//lstFileInfo.push_back(tempfile);
		lstPacket.push_back(CPacket(2, (char*)&tempfile, sizeof(tempfile)));
		//m_pServer->Send(pack);
		OutputDebugString(_T("该目录不存在！\n"));
		return -2;
	}
	//遍历该目录下的文件
	_finddata_t filedata;
	int hfind = 0;
	if ((hfind = _findfirst("*", &filedata)) == -1) {//在当前目录下查找文件或子目录，提供与参数filespec中指定的文件名匹配的第一个实例。_findfirst接收一个通配符字符串作为参数，可以用于查找文件或文件夹。
		fileinfo tempfile;
		tempfile.IsHasNext = FALSE;
		lstPacket.push_back(CPacket(2, (char*)&tempfile, sizeof(tempfile)));
		//m_pServer->Send(pack);
		OutputDebugString(_T("没有找到该文件！\n"));//输出调试模式一般在Debug模式下，不过如果不加控制条件，在release模式下也可以使用
		return -3;
	}
	int count = 0;
	do {//此时要将文件信息生成一个列表，想办法将这些文件正常提交到控制端，所以这里要再写一个结构体
		fileinfo tempfile;//临时文件
		tempfile.IsDirectory = (filedata.attrib & _A_SUBDIR) != 0;//filedata.attrib&_A_SUBDIR 判断他是否有子目录(文件夹)  TRUE表示文件夹(目录)，FALSE表示文件。
		memcpy(tempfile.szFileName, filedata.name, strlen(filedata.name));
		//lstFileInfo.push_back(tempfile);
		TRACE("serv[%s] IsDirectory:%d\r\n", tempfile.szFileName, tempfile.IsDirectory);
		lstPacket.push_back(CPacket(2, (char*)&tempfile, sizeof(tempfile)));
		//Dump((BYTE*)pack.Data(), pack.Size());
		Sleep(1);
		//m_pServer->Send(pack);
		count++;
	} while (!_findnext(hfind, &filedata));//查找下一个名称  _findnext()如果成功返回0
	//发送信息到控制端，但是如果该文件夹有大量的文件，一次性发完效果不是很好，所以要一个一个发，但是从用户体验方面来说，后者与用户有交互，可以随时反馈，所以这里选后者，结构体加一个是否还有后续的判断变量
	TRACE("服务端%s路径的%d个文件全部发送！\n", strPath.c_str(), count);
	fileinfo tempfile;
	tempfile.IsHasNext = FALSE;
	lstPacket.push_back(CPacket(2, (char*)&tempfile, sizeof(tempfile)));

	//m_pServer->Send(pack);

	return 0;
}

int CCommand::RunFile(std::list<CPacket>& lstPacket, CPacket& packet)//这里是运行而不是打开文件是因为有些文件是.exe，有些文件是默认应用打开
{
	std::string strPath = packet.strData;
	/*m_pServer->GetFilePath(strPath);*/
	HINSTANCE ret = ShellExecuteA(NULL, "open", strPath.c_str(), NULL, NULL, SW_SHOW);//对指定文件执行操作，相当于双击文件  ShellExecute是Unicode字符集的，ShellExecuteA是多字符集的，所以这里用后者
	if ((int)ret <= 32) {
		TRACE("文件打开失败:%d\n", GetLastError());
		return -1;
	}
	Sleep(1);
	lstPacket.push_back(CPacket(3, NULL, NULL));
	//m_pServer->Send(pack);
	return 0;
}

int CCommand::DownloadFile(std::list<CPacket>& lstPacket, CPacket& packet)//下载文件
{
	std::string strPath = packet.strData;
	/*m_pServer->GetFilePath(strPath);*/
	long long data = 0;
	FILE* pFile = NULL;//文件默认为NULL，因为可能存在文件打开了但读不到数据，但是fopen_s如果失败了，那pFile还是NULL.
	errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");//可读打开一个二进制文件，文本可以通过二进制方式来读，但二进制文件不能文本方式读。
	if (err != 0) {//打开失败
		lstPacket.push_back(CPacket(4, (char*)data, 8));//下载文件默认长度为0
		//m_pServer->Send(pack);
		return -1;
	}
	if (pFile != NULL) {
		fseek(pFile, 0, SEEK_END);//将文件指针移到指针位置，SEEK_END文件结尾
		data = _ftelli64(pFile);//获取指针当前位置；如果处理大型文件，就用_ftelli64，即64位有符号整数类型，可处理大于2GB的文件。
		//char* len= new char[10];
		//sprintf(len, "%lld", data);
		lstPacket.push_back(CPacket(4, (char*)&data, 8));//将要下载的文件大小发送到控制端
		//m_pServer->Send(pack);
		//delete[] len;
		fseek(pFile, 0, SEEK_SET);//恢复文件指针
		char buffer[1024] = { 0 };
		int count = 0;
		size_t rlen = 0;
		do {
			rlen = fread(buffer, 1, sizeof(buffer), pFile);//从流读取数据
			lstPacket.push_back(CPacket(4, buffer, rlen));
			Sleep(1);//等待接受的时间，否则接收不全
			//m_pServer->Send(pack);
			count += rlen;
			memset(buffer, 0, sizeof(buffer));
		} while (rlen > 0);//只要读到了就可以继续读或rlen>=1024
		TRACE("下载的文件大小:%d\n", count);
		fclose(pFile);//关闭文件
	}
	lstPacket.push_back(CPacket(4, NULL, 0));
	//m_pServer->Send(pack);
	return 0;
}

int CCommand::MouseEvent(std::list<CPacket>& lstPacket, CPacket& packet)//鼠标操作
{
	mouseev mouse;
	memcpy(&mouse, packet.strData.c_str(), sizeof(MOUSEEV));
	WORD nFlag;//2字节 0000 0000  0000 0000,设置一个标志位来处理鼠标按键信息
	switch (mouse.nButton) {
	case 0://左键
		nFlag = 1;//0001
		break;
	case 1://右键
		nFlag = 2;//0010
		break;
	case 2://中键
		nFlag = 4;//0100
		break;
	case 3://没有按键，单纯鼠标移动
		nFlag = 8;//1000
		break;
	}
	if (nFlag != 8)//没有按键的鼠标坐标搞定了，其余有按键时鼠标坐标在这里搞
		SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//将光标移动到指定的屏幕坐标
	switch (mouse.nAction) {
	case 0://单击
		nFlag |= 0X10;//0001 0000
		break;
	case 1://双击
		nFlag |= 0X20;//0010 0000
		break;
	case 2://按下
		nFlag |= 0X40;//0100 0000
		break;
	case 4://放开
		nFlag |= 0X80;//1000 0000
		break;
	default://没有按键直接退出
		break;
	}
	switch (nFlag) {
	case 0X21://左键双击
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());//鼠标运动和按钮单击,GetMessageExtraInfo()与鼠标事件关联的额外消息信息
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
	case 0X11://左键单击
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		break;//单击就执行1次按下、放开，双击就执行2次
	case 0X41://左键按下
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X81://左键放开
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		break;

	case 0X22://右键双击
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
	case 0X12://右键单击
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X42://右键按下
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X82://右键放开
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		break;

	case 0X24://中键双击
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
	case 0X14://中键单击
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X44://中键按下
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X84://中键放开
		mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		break;

	case 0X08://鼠标移动
		//mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
		SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//将光标移动到指定的屏幕坐标
		break;
	}
	//鼠标操作处理完后，发消息验证一下
	lstPacket.push_back(CPacket(4, NULL, 0));
	//m_pServer->Send(pack);
	return 0;
}

int CCommand::SendScreen(std::list<CPacket>& lstPacket,CPacket& packet)//远程监控，屏幕截图发送给控制端，这里不需要从控制端获取数据
{
	CImage screen;//提供增强的位图支持，能够加载和保存JPEG、GIF、BMP和可移植网络图形格式PNG的图像。CImage里面封装了很多关于图形图像的操作，非常适合Windows下的GDI编程(图形设备接口)
	//获取屏幕的句柄
	HDC hdcScreen = ::GetDC(NULL);//检索指定窗口或整个屏幕工作区的设备上下文(DC)的句柄。NULL表示检索整个屏幕的DC
	int nBitPerPixel = GetDeviceCaps(hdcScreen, BITSPIXEL);//检索指定屏幕的特定信息  BITSPIXEL指每个像素相邻的颜色位数
	int nWidth = GetDeviceCaps(hdcScreen, HORZRES);//屏幕的宽度，以像素为单位
	int nHeight = GetDeviceCaps(hdcScreen, VERTRES);//屏幕的高度，以光栅线为单位
	screen.Create(nWidth, nHeight, nBitPerPixel);//创建CImage位图并将其附加到先前构造的CImage对象
	BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hdcScreen, 0, 0, SRCCOPY);//从指定源设备上下文到目标设备上下文中的像素矩形对应的颜色数据的位块传输；screen.GetDC()检索当前已选择图像的设备上下文；SRCCOPY 将源矩形直接复制到目标矩形
	ReleaseDC(NULL, hdcScreen);//释放源设备上下文，与GetDC对应
	screen.ReleaseDC();//释放使用 CImage 类检索的目标设备上下文。

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//用来存储流内容；从堆分配指定的字节数，全局内存对象句柄，GMEM_MOVEABLE 分配可移动内存;
	if (hMem == NULL) {
		TRACE("分配移动内存失败errno=%d\n", GetLastError());
		return -1;
	}
	IStream* pStream = NULL;//新的流对象接口指针
	HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//创建一个流对象，使用HGLOBAL内存句柄来存储流内容，接口指向IStream* 的地址；TRUR自动释放流对象的基础句柄。
	if (ret == S_OK) {//S_OK函数执行成功
		screen.Save(pStream, Gdiplus::ImageFormatPNG);//将图片存储到流中
		LARGE_INTEGER bg = { 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);//将查找指针更改为新位置。bg表示流对象指针的偏移量
		PBYTE pdata = (PBYTE)GlobalLock(hMem);//返回该对象内存块的第一个字节的指针。
		SIZE_T nSize = GlobalSize(hMem);//检索全局内存对象的当前大小(以字节为单位)
		lstPacket.push_back(CPacket(6, (const char*)pdata, nSize));
		//m_pServer->Send(pack);
	}
	GlobalUnlock(hMem);//用于解锁由GlobalLock 函数锁定的内存块
	pStream->Release(); //用于释放IStream 接口对象所占用的资源

	//screen.Save(_T("屏幕截图.png"), Gdiplus::ImageFormatPNG);//网络的稳定性更重要，png图像更准确，细节，无损压缩，这里选择用png图像

	//ULONGLONG tick=GetTickCount64();//GetTickCount64()检索自系统启动以来已用过的毫秒数。相比于GetTickCount，GetTickCount64可提供更长的计时范围。
	//screen.Save(_T("屏幕截图.jpg"),Gdiplus::ImageFormatJPEG);//将图像另存为指定类型；这里填的是屏幕截图的命名，而不是路径
	//TRACE("jpg:%d\n", GetTickCount64() - tick);

	return 0;
}

int CCommand::DelFile(std::list<CPacket>& lstPacket, CPacket& packet)//删除文件
{
	std::string strPath=packet.strData;
	TCHAR sPath[MAX_PATH] = _T("");
	//mbstowcs(sPath, strPath.c_str(), strPath.size());//将多字节字符序列转换为对应的宽字符序列
	MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));//将字符串映射到UTF-16(宽字符)字符串，字符串不一定来自多字节字符串。
	if (DeleteFile(sPath)) {
		OutputDebugString(_T("文件删除成功"));
		char data[] = "删除文件成功!";
		lstPacket.push_back(CPacket(7, data, strlen(data)));
		//m_pServer->Send(pack);
	}
	else {
		OutputDebugString(_T("文件删除失败"));
		char data[] = "删除文件失败!";
		lstPacket.push_back(CPacket(7, data, strlen(data)));
		//m_pServer->Send(pack);
	}
	//if (std::remove(strPath.c_str()) == 0) {//删除文件
	//	OutputDebugString(_T("文件删除成功"));
	//	char data[] = "删除文件成功!";
	//	CPacket pack(7, data, strlen(data));
	//	pServer->Send(pack);
	//}
	//else {
	//	OutputDebugString(_T("文件删除失败"));
	//	char data[] = "删除文件失败!";
	//	CPacket pack(7, data, strlen(data));
	//	pServer->Send(pack);
	//}
	return 0;
}

//unsigned __stdcall threadLockDlg(void* arg);

int CCommand::LockMachine(std::list<CPacket>& lstPacket,CPacket& packet)//锁机
{
	if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {//锁机窗口不存在或创建失败；防止锁机重复执行
		_beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);//防止网络通信的时候锁机不会阻塞在这，导致无法退出循环接收外部解锁命令
	}
	else {
		TRACE("锁机重复了");
	}
	lstPacket.push_back(CPacket(8, NULL, 0));
	//m_pServer->Send(pack);
	Sleep(50);//等待当前线程的执行
	return 0;
}

int CCommand::UnLockMachine(std::list<CPacket>& lstPacket, CPacket& packet)//解锁
{
	//dlg.SendMessage(WM_KEYDOWN, 0X1B, 0X10001);
	PostThreadMessage(threadid, WM_KEYDOWN, 0X1B, 0X10001);//将消息发送到指定线程的消息队列,发送按下"Esc"键解锁。
	lstPacket.push_back(CPacket(9, NULL, 0));
	//m_pServer->Send(pack);
	while (dlg.m_hWnd != NULL) {
		Sleep(100);//等待线程执行结束；不能直接退出，因为线程还未析构
	}
	return 0;
}

int CCommand::ConnectTect(std::list<CPacket>& lstPacket, CPacket& packet)
{
	lstPacket.push_back(CPacket(1981, NULL, 0));
	//m_pServer->Send(pack);
	TRACE("1981连接测试成功\n");
	return 0;
}

void CCommand::Lock()
{
	//创建（初始化）对话框
	dlg.Create(IDD_DIALOG_LOCK, NULL);//因为如果被销毁了，下次在想锁机的话，该对话框就创建不了了，所以将对话框的创建放在锁机函数里
	dlg.ShowWindow(SW_SHOW);//显示窗口，设置窗口的可见性
	//设置窗口大小
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//检索主显示器全屏窗口工作区宽度
	rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) + 30;//检索主显示器全屏窗口工作区高度(以像素为单位)
	TRACE("right:%d  bottom:%d\n", rect.right, rect.bottom);
	dlg.MoveWindow(&rect);//更改窗口的位置和尺寸

	//将锁机后的文本居中
	CWnd* m_text = dlg.GetDlgItem(IDC_STATIC);//通过ID检索对话框或控件的指针
	if (m_text != NULL) {
		CRect rText;
		m_text->GetWindowRect(rText);//获取对象边框的尺寸
		int x = (rect.right - rText.Width()) / 2;//锁机文本左上角x坐标
		int y = (rect.bottom - rText.Height()) / 2;//锁机文本左上角y坐标
		m_text->MoveWindow(x, y, rText.Width(), rText.Height());
	}

	//窗口置顶
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//更改子窗口的位置，将它设置到最顶部,SWP_NOSIZE 保留当前大小, SWP_NOMOVE 保留当前位置
	//这是没有显示窗口是因为缺少消息循环
	dlg.UpdateWindow();//刷新窗口
	//隐藏任务栏和鼠标
	ShowWindow(FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
	ShowCursor(FALSE);//隐藏鼠标，锁机之前隐藏鼠标，锁机之后显示鼠标
	//限制鼠标的活动范围
	//dlg.GetWindowRect(&rect);//获取CWnd对话框的屏幕坐标
	rect.left = rect.left + 1;//左上角x坐标
	rect.right = rect.left + 1;//右下角x坐标
	rect.bottom = rect.top + 1;//右下角y坐标
	rect.top = rect.top + 1;//左上角y坐标
	ClipCursor(&rect);//限制鼠标的活动范围
	//对话框消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);//将虚拟秘钥消息转换为字符消息
		DispatchMessage(&msg);//分配消息给对应的窗口进行处理
		if (msg.message == WM_KEYDOWN) {//WM_KEYDOWN 按下非系统键(未按下Alt键时按下的键)
			TRACE("message:%08X  wParam:%08X  lParam:%08X\n", msg.message, msg.wParam, msg.lParam);//08X表示以16进制输出8个字符
			if (0X1B == msg.wParam) {//只有按键为Esc键时才会销毁窗口。

				break;
			}
		}
	}
	dlg.DestroyWindow();//销毁窗口
	ClipCursor(NULL);//恢复鼠标的活动范围
	ShowCursor(TRUE);//显示鼠标
	ShowWindow(FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);//显示任务栏
}

unsigned __stdcall CCommand::threadLockDlg(void* arg)//锁机不能放在主线程里，否则程序会一直循环在锁机函数里，外部接收不到解锁命令，所以这里将锁机部分放在子线程里
{
	CCommand* thiz = (CCommand*)arg;
	thiz->Lock();
	_endthreadex(0);//终止由_beginthreadex 创建的线程
	return 0;
}

