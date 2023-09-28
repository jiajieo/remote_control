#include "pch.h"
#include "Command.h"
#include <direct.h>
#include "Tool.h"
#include <io.h>
#include <atlimage.h>
#include "Resource.h"

CLockDialog dlg;//������ȫ�֣�Ϊ��ȷ���������򶼿��Է��ʸöԻ���

CCommand::CCommand()
{
	struct {
		int nCmd;
		CMDFUNC func;
	}data[] = {//����data�Ǳ�����������ʼ��
		{1,&CCommand::MakeDriverInfo},//�鿴���̷���
		{2,&CCommand::MakeDirectoryInfo},//�鿴ָ��Ŀ¼�µ��ļ�
		{3,&CCommand::RunFile},//���ļ�
		{4,&CCommand::DownloadFile},//�����ļ�
		{5,&CCommand::MouseEvent},//������
		{6,&CCommand::SendScreen},//Զ�̼�أ�������Ļ��ͼ�����ƶ�
		{7,&CCommand::DelFile},//ɾ���ļ�
		{8,&CCommand::LockMachine},//����
		{9,&CCommand::UnLockMachine},//����
		{1981,&CCommand::ConnectTect},//��������
		{-1,NULL}
	};
	for (int i = 0; data[i].nCmd != -1; i++) {
		m_mapFunction.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func));//����ʼ���ļ�ֵ�Զ����뵽map������
	}
}

int CCommand::ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& packet)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);//����һ���������ܹ�ȡ��map�������������
	if (it == m_mapFunction.end())//m_mapFunction.end()����ָ������ĩβ�ĵ�����
	{//û�ҵ�������
		return -1;
	}
	//�ҵ�������ͷ��ظ�Ԫ�صĳ�Ա����ָ�����
	return (this->*(it->second))(lstPacket, packet);//this->*��ȡ���ҵ�һ����Ա
}

void CCommand::RunCCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& packet)//arg���ظ���Ķ���status�ǽ��յ�������
{
	CCommand* thiz = (CCommand*)arg;
	if (status > 0) {
		if (thiz->ExcuteCommand(status, lstPacket, packet) != 0) {//����ִ�гɹ��᷵��0
			TRACE("ִ������ʧ��:sCmd=%d\n", status);
		}
	}
	else {
		MessageBox(NULL, _T("���������ʧ�ܣ��Զ����ԣ�"), _T("����"), MB_OK | MB_ICONERROR);//NULL��ʾ���յ�������
	}
}


//int CCommand::ExcuteCommand(int nCmd)
//{
//	int ret;
//	switch (nCmd) {
//	case 1://�鿴���̷���
//		ret = MakeDriverInfo();
//		break;
//	case 2://�鿴ָ��Ŀ¼�µ��ļ�
//		ret = MakeDirectoryInfo();
//		break;
//	case 3://���ļ�
//		ret = RunFile();
//		break;
//	case 4://�����ļ�
//		ret = DownloadFile();
//		break;
//	case 5://������
//		ret = MouseEvent();
//		break;
//	case 6://Զ�̼�أ�������Ļ��ͼ�����ƶ�
//		ret = SendScreen();
//		break;
//	case 7://ɾ���ļ�
//		ret = DelFile();
//		break;
//	case 8://����
//		ret = LockMachine();
//		Sleep(50);//�ȴ���ǰ�̵߳�ִ��
//		//LockMachine();
//		break;
//	case 9://����
//		ret = UnLockMachine();
//		while (dlg.m_hWnd != NULL) {
//			Sleep(100);//�ȴ��߳�ִ�н���������ֱ���˳�����Ϊ�̻߳�δ����
//		}
//		break;
//	case 1981://��������
//		ConnectTect();
//		break;
//	}
//	//Sleep(5000);//�ȴ���ǰ�����߳�ִ��5s
//	//UnLockMachine();//����
//	return 0;
//}

int CCommand::MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& packet)//����һ�����̷���
{
	std::string result;
	for (int i = 1; i <= 26; i++) {//���Ҵ��̷���
		if (_chdrive(i) == 0) {//���ĵ�ǰ�Ĺ��������� 1=>(����)A�� 2=>B�� ... ��ൽ26=>Z��
			//���ڵĴ���
			if (result.size() > 0) {
				result += ',';//��ÿ��������","�ָ���
			}
			result += 'A' + i - 1;//string���ַ�������ֱ��ͨ��+�ķ�ʽ���ַ�����ƴ��
		}
	}
	//�����ݽ��д��
	lstPacket.push_back(CPacket(1, result.c_str(), result.size()));//��β�����뵽��������
	//CTool::Dump((BYTE*)pack.Data(), pack.Size());
	//�õ�ʵ����������
	//m_pServer->Send(pack);
	return 0;
}

int CCommand::MakeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& packet)//�鿴ָ��Ŀ¼�µ��ļ�
{
	std::string strPath = packet.strData;
	//std::list<FILEINFO> lstFileInfo;//�øýṹ�嶨��һ�����������ļ�����
	/*if (m_pServer->GetFilePath(strPath) == false) {
		OutputDebugString(_T("���󣬵�ǰ����ǻ�ȡ�ļ�·�������\n"));
		return -1;
	}*/
	//���ݻ�ȡ�ɹ����鿴�ļ�����
	if (_chdir(strPath.c_str()) != 0) {//���ĵ�ǰ����Ŀ¼������ָ��Ŀ¼����_findfirst�����ʹ��
		//Ŀ¼�����ڵ����
		fileinfo tempfile;
		tempfile.IsInvalid = TRUE;//Ŀ¼��Ч
		tempfile.IsHasNext = FALSE;
		//lstFileInfo.push_back(tempfile);
		lstPacket.push_back(CPacket(2, (char*)&tempfile, sizeof(tempfile)));
		//m_pServer->Send(pack);
		OutputDebugString(_T("��Ŀ¼�����ڣ�\n"));
		return -2;
	}
	//������Ŀ¼�µ��ļ�
	_finddata_t filedata;
	int hfind = 0;
	if ((hfind = _findfirst("*", &filedata)) == -1) {//�ڵ�ǰĿ¼�²����ļ�����Ŀ¼���ṩ�����filespec��ָ�����ļ���ƥ��ĵ�һ��ʵ����_findfirst����һ��ͨ����ַ�����Ϊ�������������ڲ����ļ����ļ��С�
		fileinfo tempfile;
		tempfile.IsHasNext = FALSE;
		lstPacket.push_back(CPacket(2, (char*)&tempfile, sizeof(tempfile)));
		//m_pServer->Send(pack);
		OutputDebugString(_T("û���ҵ����ļ���\n"));//�������ģʽһ����Debugģʽ�£�����������ӿ�����������releaseģʽ��Ҳ����ʹ��
		return -3;
	}
	int count = 0;
	do {//��ʱҪ���ļ���Ϣ����һ���б���취����Щ�ļ������ύ�����ƶˣ���������Ҫ��дһ���ṹ��
		fileinfo tempfile;//��ʱ�ļ�
		tempfile.IsDirectory = (filedata.attrib & _A_SUBDIR) != 0;//filedata.attrib&_A_SUBDIR �ж����Ƿ�����Ŀ¼(�ļ���)  TRUE��ʾ�ļ���(Ŀ¼)��FALSE��ʾ�ļ���
		memcpy(tempfile.szFileName, filedata.name, strlen(filedata.name));
		//lstFileInfo.push_back(tempfile);
		TRACE("serv[%s] IsDirectory:%d\r\n", tempfile.szFileName, tempfile.IsDirectory);
		lstPacket.push_back(CPacket(2, (char*)&tempfile, sizeof(tempfile)));
		//Dump((BYTE*)pack.Data(), pack.Size());
		Sleep(1);
		//m_pServer->Send(pack);
		count++;
	} while (!_findnext(hfind, &filedata));//������һ������  _findnext()����ɹ�����0
	//������Ϣ�����ƶˣ�����������ļ����д������ļ���һ���Է���Ч�����Ǻܺã�����Ҫһ��һ���������Ǵ��û����鷽����˵���������û��н�����������ʱ��������������ѡ���ߣ��ṹ���һ���Ƿ��к������жϱ���
	TRACE("�����%s·����%d���ļ�ȫ�����ͣ�\n", strPath.c_str(), count);
	fileinfo tempfile;
	tempfile.IsHasNext = FALSE;
	lstPacket.push_back(CPacket(2, (char*)&tempfile, sizeof(tempfile)));

	//m_pServer->Send(pack);

	return 0;
}

int CCommand::RunFile(std::list<CPacket>& lstPacket, CPacket& packet)//���������ж����Ǵ��ļ�����Ϊ��Щ�ļ���.exe����Щ�ļ���Ĭ��Ӧ�ô�
{
	std::string strPath = packet.strData;
	/*m_pServer->GetFilePath(strPath);*/
	HINSTANCE ret = ShellExecuteA(NULL, "open", strPath.c_str(), NULL, NULL, SW_SHOW);//��ָ���ļ�ִ�в������൱��˫���ļ�  ShellExecute��Unicode�ַ����ģ�ShellExecuteA�Ƕ��ַ����ģ����������ú���
	if ((int)ret <= 32) {
		TRACE("�ļ���ʧ��:%d\n", GetLastError());
		return -1;
	}
	Sleep(1);
	lstPacket.push_back(CPacket(3, NULL, NULL));
	//m_pServer->Send(pack);
	return 0;
}

int CCommand::DownloadFile(std::list<CPacket>& lstPacket, CPacket& packet)//�����ļ�
{
	std::string strPath = packet.strData;
	/*m_pServer->GetFilePath(strPath);*/
	long long data = 0;
	FILE* pFile = NULL;//�ļ�Ĭ��ΪNULL����Ϊ���ܴ����ļ����˵����������ݣ�����fopen_s���ʧ���ˣ���pFile����NULL.
	errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");//�ɶ���һ���������ļ����ı�����ͨ�������Ʒ�ʽ���������������ļ������ı���ʽ����
	if (err != 0) {//��ʧ��
		lstPacket.push_back(CPacket(4, (char*)data, 8));//�����ļ�Ĭ�ϳ���Ϊ0
		//m_pServer->Send(pack);
		return -1;
	}
	if (pFile != NULL) {
		fseek(pFile, 0, SEEK_END);//���ļ�ָ���Ƶ�ָ��λ�ã�SEEK_END�ļ���β
		data = _ftelli64(pFile);//��ȡָ�뵱ǰλ�ã������������ļ�������_ftelli64����64λ�з����������ͣ��ɴ������2GB���ļ���
		//char* len= new char[10];
		//sprintf(len, "%lld", data);
		lstPacket.push_back(CPacket(4, (char*)&data, 8));//��Ҫ���ص��ļ���С���͵����ƶ�
		//m_pServer->Send(pack);
		//delete[] len;
		fseek(pFile, 0, SEEK_SET);//�ָ��ļ�ָ��
		char buffer[1024] = { 0 };
		int count = 0;
		size_t rlen = 0;
		do {
			rlen = fread(buffer, 1, sizeof(buffer), pFile);//������ȡ����
			lstPacket.push_back(CPacket(4, buffer, rlen));
			Sleep(1);//�ȴ����ܵ�ʱ�䣬������ղ�ȫ
			//m_pServer->Send(pack);
			count += rlen;
			memset(buffer, 0, sizeof(buffer));
		} while (rlen > 0);//ֻҪ�����˾Ϳ��Լ�������rlen>=1024
		TRACE("���ص��ļ���С:%d\n", count);
		fclose(pFile);//�ر��ļ�
	}
	lstPacket.push_back(CPacket(4, NULL, 0));
	//m_pServer->Send(pack);
	return 0;
}

int CCommand::MouseEvent(std::list<CPacket>& lstPacket, CPacket& packet)//������
{
	mouseev mouse;
	memcpy(&mouse, packet.strData.c_str(), sizeof(MOUSEEV));
	WORD nFlag;//2�ֽ� 0000 0000  0000 0000,����һ����־λ��������갴����Ϣ
	switch (mouse.nButton) {
	case 0://���
		nFlag = 1;//0001
		break;
	case 1://�Ҽ�
		nFlag = 2;//0010
		break;
	case 2://�м�
		nFlag = 4;//0100
		break;
	case 3://û�а�������������ƶ�
		nFlag = 8;//1000
		break;
	}
	if (nFlag != 8)//û�а������������㶨�ˣ������а���ʱ��������������
		SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//������ƶ���ָ������Ļ����
	switch (mouse.nAction) {
	case 0://����
		nFlag |= 0X10;//0001 0000
		break;
	case 1://˫��
		nFlag |= 0X20;//0010 0000
		break;
	case 2://����
		nFlag |= 0X40;//0100 0000
		break;
	case 4://�ſ�
		nFlag |= 0X80;//1000 0000
		break;
	default://û�а���ֱ���˳�
		break;
	}
	switch (nFlag) {
	case 0X21://���˫��
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());//����˶��Ͱ�ť����,GetMessageExtraInfo()������¼������Ķ�����Ϣ��Ϣ
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
	case 0X11://�������
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		break;//������ִ��1�ΰ��¡��ſ���˫����ִ��2��
	case 0X41://�������
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X81://����ſ�
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		break;

	case 0X22://�Ҽ�˫��
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
	case 0X12://�Ҽ�����
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X42://�Ҽ�����
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X82://�Ҽ��ſ�
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		break;

	case 0X24://�м�˫��
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
	case 0X14://�м�����
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
		mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X44://�м�����
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
		break;
	case 0X84://�м��ſ�
		mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		break;

	case 0X08://����ƶ�
		//mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
		SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//������ƶ���ָ������Ļ����
		break;
	}
	//������������󣬷���Ϣ��֤һ��
	lstPacket.push_back(CPacket(5, NULL, 0));
	//m_pServer->Send(pack);
	return 0;
}

int CCommand::SendScreen(std::list<CPacket>& lstPacket,CPacket& packet)//Զ�̼�أ���Ļ��ͼ���͸����ƶˣ����ﲻ��Ҫ�ӿ��ƶ˻�ȡ����
{
	CImage screen;//�ṩ��ǿ��λͼ֧�֣��ܹ����غͱ���JPEG��GIF��BMP�Ϳ���ֲ����ͼ�θ�ʽPNG��ͼ��CImage�����װ�˺ܶ����ͼ��ͼ��Ĳ������ǳ��ʺ�Windows�µ�GDI���(ͼ���豸�ӿ�)
	//��ȡ��Ļ�ľ��
	HDC hdcScreen = ::GetDC(NULL);//����ָ�����ڻ�������Ļ���������豸������(DC)�ľ����NULL��ʾ����������Ļ��DC
	int nBitPerPixel = GetDeviceCaps(hdcScreen, BITSPIXEL);//����ָ����Ļ���ض���Ϣ  BITSPIXELָÿ���������ڵ���ɫλ��
	int nWidth = GetDeviceCaps(hdcScreen, HORZRES);//��Ļ�Ŀ�ȣ�������Ϊ��λ
	int nHeight = GetDeviceCaps(hdcScreen, VERTRES);//��Ļ�ĸ߶ȣ��Թ�դ��Ϊ��λ
	screen.Create(nWidth, nHeight, nBitPerPixel);//����CImageλͼ�����丽�ӵ���ǰ�����CImage����
	BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hdcScreen, 0, 0, SRCCOPY);//��ָ��Դ�豸�����ĵ�Ŀ���豸�������е����ؾ��ζ�Ӧ����ɫ���ݵ�λ�鴫�䣻screen.GetDC()������ǰ��ѡ��ͼ����豸�����ģ�SRCCOPY ��Դ����ֱ�Ӹ��Ƶ�Ŀ�����
	ReleaseDC(NULL, hdcScreen);//�ͷ�Դ�豸�����ģ���GetDC��Ӧ
	screen.ReleaseDC();//�ͷ�ʹ�� CImage �������Ŀ���豸�����ġ�

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//�����洢�����ݣ��Ӷѷ���ָ�����ֽ�����ȫ���ڴ��������GMEM_MOVEABLE ������ƶ��ڴ�;
	if (hMem == NULL) {
		TRACE("�����ƶ��ڴ�ʧ��errno=%d\n", GetLastError());
		return -1;
	}
	IStream* pStream = NULL;//�µ�������ӿ�ָ��
	HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//����һ��������ʹ��HGLOBAL�ڴ������洢�����ݣ��ӿ�ָ��IStream* �ĵ�ַ��TRUR�Զ��ͷ�������Ļ��������
	if (ret == S_OK) {//S_OK����ִ�гɹ�
		screen.Save(pStream, Gdiplus::ImageFormatPNG);//��ͼƬ�洢������
		LARGE_INTEGER bg = { 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);//������ָ�����Ϊ��λ�á�bg��ʾ������ָ���ƫ����
		PBYTE pdata = (PBYTE)GlobalLock(hMem);//���ظö����ڴ��ĵ�һ���ֽڵ�ָ�롣
		SIZE_T nSize = GlobalSize(hMem);//����ȫ���ڴ����ĵ�ǰ��С(���ֽ�Ϊ��λ)
		lstPacket.push_back(CPacket(6, (const char*)pdata, nSize));
		//m_pServer->Send(pack);
	}
	GlobalUnlock(hMem);//���ڽ�����GlobalLock �����������ڴ��
	pStream->Release(); //�����ͷ�IStream �ӿڶ�����ռ�õ���Դ

	//screen.Save(_T("��Ļ��ͼ.png"), Gdiplus::ImageFormatPNG);//������ȶ��Ը���Ҫ��pngͼ���׼ȷ��ϸ�ڣ�����ѹ��������ѡ����pngͼ��

	//ULONGLONG tick=GetTickCount64();//GetTickCount64()������ϵͳ�����������ù��ĺ������������GetTickCount��GetTickCount64���ṩ�����ļ�ʱ��Χ��
	//screen.Save(_T("��Ļ��ͼ.jpg"),Gdiplus::ImageFormatJPEG);//��ͼ�����Ϊָ�����ͣ������������Ļ��ͼ��������������·��
	//TRACE("jpg:%d\n", GetTickCount64() - tick);

	return 0;
}

int CCommand::DelFile(std::list<CPacket>& lstPacket, CPacket& packet)//ɾ���ļ�
{
	std::string strPath=packet.strData;
	TCHAR sPath[MAX_PATH] = _T("");
	//mbstowcs(sPath, strPath.c_str(), strPath.size());//�����ֽ��ַ�����ת��Ϊ��Ӧ�Ŀ��ַ�����
	MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));//���ַ���ӳ�䵽UTF-16(���ַ�)�ַ������ַ�����һ�����Զ��ֽ��ַ�����
	if (DeleteFile(sPath)) {
		OutputDebugString(_T("�ļ�ɾ���ɹ�"));
		char data[] = "ɾ���ļ��ɹ�!";
		lstPacket.push_back(CPacket(7, data, strlen(data)));
		//m_pServer->Send(pack);
	}
	else {
		OutputDebugString(_T("�ļ�ɾ��ʧ��"));
		char data[] = "ɾ���ļ�ʧ��!";
		lstPacket.push_back(CPacket(7, data, strlen(data)));
		//m_pServer->Send(pack);
	}
	//if (std::remove(strPath.c_str()) == 0) {//ɾ���ļ�
	//	OutputDebugString(_T("�ļ�ɾ���ɹ�"));
	//	char data[] = "ɾ���ļ��ɹ�!";
	//	CPacket pack(7, data, strlen(data));
	//	pServer->Send(pack);
	//}
	//else {
	//	OutputDebugString(_T("�ļ�ɾ��ʧ��"));
	//	char data[] = "ɾ���ļ�ʧ��!";
	//	CPacket pack(7, data, strlen(data));
	//	pServer->Send(pack);
	//}
	return 0;
}

//unsigned __stdcall threadLockDlg(void* arg);

int CCommand::LockMachine(std::list<CPacket>& lstPacket,CPacket& packet)//����
{
	if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {//�������ڲ����ڻ򴴽�ʧ�ܣ���ֹ�����ظ�ִ��
		_beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);//��ֹ����ͨ�ŵ�ʱ�����������������⣬�����޷��˳�ѭ�������ⲿ��������
	}
	else {
		TRACE("�����ظ���");
	}
	lstPacket.push_back(CPacket(8, NULL, 0));
	//m_pServer->Send(pack);
	Sleep(50);//�ȴ���ǰ�̵߳�ִ��
	return 0;
}

int CCommand::UnLockMachine(std::list<CPacket>& lstPacket, CPacket& packet)//����
{
	//dlg.SendMessage(WM_KEYDOWN, 0X1B, 0X10001);
	PostThreadMessage(threadid, WM_KEYDOWN, 0X1B, 0X10001);//����Ϣ���͵�ָ���̵߳���Ϣ����,���Ͱ���"Esc"��������
	lstPacket.push_back(CPacket(9, NULL, 0));
	//m_pServer->Send(pack);
	while (dlg.m_hWnd != NULL) {
		Sleep(100);//�ȴ��߳�ִ�н���������ֱ���˳�����Ϊ�̻߳�δ����
	}
	return 0;
}

int CCommand::ConnectTect(std::list<CPacket>& lstPacket, CPacket& packet)
{
	lstPacket.push_back(CPacket(1981, NULL, 0));
	//m_pServer->Send(pack);
	TRACE("1981���Ӳ��Գɹ�\n");
	return 0;
}

void CCommand::Lock()
{
	//��������ʼ�����Ի���
	dlg.Create(IDD_DIALOG_LOCK, NULL);//��Ϊ����������ˣ��´����������Ļ����öԻ���ʹ��������ˣ����Խ��Ի���Ĵ�����������������
	dlg.ShowWindow(SW_SHOW);//��ʾ���ڣ����ô��ڵĿɼ���
	//���ô��ڴ�С
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//��������ʾ��ȫ�����ڹ��������
	rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) + 30;//��������ʾ��ȫ�����ڹ������߶�(������Ϊ��λ)
	TRACE("right:%d  bottom:%d\n", rect.right, rect.bottom);
	dlg.MoveWindow(&rect);//���Ĵ��ڵ�λ�úͳߴ�

	//����������ı�����
	CWnd* m_text = dlg.GetDlgItem(IDC_STATIC);//ͨ��ID�����Ի����ؼ���ָ��
	if (m_text != NULL) {
		CRect rText;
		m_text->GetWindowRect(rText);//��ȡ����߿�ĳߴ�
		int x = (rect.right - rText.Width()) / 2;//�����ı����Ͻ�x����
		int y = (rect.bottom - rText.Height()) / 2;//�����ı����Ͻ�y����
		m_text->MoveWindow(x, y, rText.Width(), rText.Height());
	}

	//�����ö�
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//�����Ӵ��ڵ�λ�ã��������õ����,SWP_NOSIZE ������ǰ��С, SWP_NOMOVE ������ǰλ��
	//����û����ʾ��������Ϊȱ����Ϣѭ��
	dlg.UpdateWindow();//ˢ�´���
	//���������������
	ShowWindow(FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
	ShowCursor(FALSE);//������꣬����֮ǰ������꣬����֮����ʾ���
	//�������Ļ��Χ
	//dlg.GetWindowRect(&rect);//��ȡCWnd�Ի������Ļ����
	rect.left = rect.left + 1;//���Ͻ�x����
	rect.right = rect.left + 1;//���½�x����
	rect.bottom = rect.top + 1;//���½�y����
	rect.top = rect.top + 1;//���Ͻ�y����
	ClipCursor(&rect);//�������Ļ��Χ
	//�Ի�����Ϣѭ��
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);//��������Կ��Ϣת��Ϊ�ַ���Ϣ
		DispatchMessage(&msg);//������Ϣ����Ӧ�Ĵ��ڽ��д���
		if (msg.message == WM_KEYDOWN) {//WM_KEYDOWN ���·�ϵͳ��(δ����Alt��ʱ���µļ�)
			TRACE("message:%08X  wParam:%08X  lParam:%08X\n", msg.message, msg.wParam, msg.lParam);//08X��ʾ��16�������8���ַ�
			if (0X1B == msg.wParam) {//ֻ�а���ΪEsc��ʱ�Ż����ٴ��ڡ�

				break;
			}
		}
	}
	dlg.DestroyWindow();//���ٴ���
	ClipCursor(NULL);//�ָ����Ļ��Χ
	ShowCursor(TRUE);//��ʾ���
	ShowWindow(FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);//��ʾ������
}

unsigned __stdcall CCommand::threadLockDlg(void* arg)//�������ܷ������߳����������һֱѭ��������������ⲿ���ղ�����������������ｫ�������ַ������߳���
{
	CCommand* thiz = (CCommand*)arg;
	thiz->Lock();
	_endthreadex(0);//��ֹ��_beginthreadex �������߳�
	return 0;
}

