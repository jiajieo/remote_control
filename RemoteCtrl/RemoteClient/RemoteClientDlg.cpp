
// RemoteClientDlg.cpp: 实现文件
//


#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)//用于对话框数据的交换和验证，由 UpdateData() 调用
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_servaddress(0)
	, m_port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);//AfxGetApp 返回的指针用于访问应用程序信息  LoadIcon加载图标资源
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)//用于对话框数据的交换和验证，由 UpdateData() 调用
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS, m_servaddress);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDX_Control(pDX, IDC_TREE_DIR, m_tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CRemoteClientDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_VIEWFILE, &CRemoteClientDlg::OnBnClickedBtnViewfile)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_OPEN_FILE, &CRemoteClientDlg::OnOpenFile)
	ON_MESSAGE(WM_SEND_PACKET, &CRemoteClientDlg::OnSendPacket)
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()//创建对话框时，该函数就会被调用；可用该函数初始化对话框的数据及内容
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();//将控件的值赋给成员变量
	m_servaddress = 0x7F000001;//127.0.0.1
	m_port = "6000";
	UpdateData(FALSE);//将成员变量的值赋给控件

	m_status.Create(IDD_DLG_STATUS, this);
	//m_status.ShowWindow(SW_HIDE);//隐藏此窗口

	//屏幕监控
	m_isFull = false;//数据缓存初始化

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)//当用户从控件菜单上选择命令时，函数被调用，例如最大最小化按钮，滑轮滚动等.
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();//调用对话框完成后返回
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()//窗口每次重绘产生的消息
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);//将指定的消息发送到一个或多个窗口  dc.GetSafeHdc()返回输出设备上下文

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteClientDlg::OnBnClickedBtnConnect()//连接测试
{
	// TODO: 在此添加控件通知处理程序代码
	SendPacket(1981);
	if (m_hSocket->Getpacket().sCmd == 1981)
		MessageBox("连接成功");
}


void CRemoteClientDlg::OnBnClickedBtnViewfile()//查看文件
{
	// TODO: 在此添加控件通知处理程序代码
	SendPacket(1);
	std::string drivers = m_hSocket->Getpacket().strData;
	std::string dr;
	TRACE("drivers.c_str():%s\n", drivers.c_str());
	m_tree.DeleteAllItems();//删除树视图控件的所有项
	for (size_t i = 0; i < drivers.size(); i++) {
		if (drivers[i] == ',') {
			HTREEITEM hTemp = m_tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);//在树视图控件中插入某个新项  TVI_ROOT表示树形视图的根节点，TVI_LAST表示树形视图中的最后一个节点
			m_tree.InsertItem(NULL, hTemp, TVI_LAST);
			dr.clear();//清除字符串元素
			continue;
		}
		dr += drivers[i];
		dr += ":";
	}
	HTREEITEM hTemp = m_tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
	m_tree.InsertItem(NULL, hTemp, TVI_LAST);
}

int CRemoteClientDlg::SendPacket(WORD nCmd, BYTE* pData, size_t nSize, BOOL bAutoClose)//发送命令 
{
	UpdateData();//检索控件中的数据，把控件的值赋给成员变量；FALSE将成员变量的值赋给控件
	int port = atoi(m_port);//将字符串转换为整数
	int ret;
	m_hSocket = CClientSocket::getInstance();
	if (m_hSocket != NULL) {
		if (m_hSocket->InitSocket(m_servaddress, port) == true) {
			CPacket pack(nCmd, (const char*)pData, nSize);
			if (!m_hSocket->Send(pack))
				return -1;
			ret = m_hSocket->Recv();
			if (bAutoClose)//bAutoClose默认为TRUE
				m_hSocket->CloseSocket();
		}
	}
	return ret;
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree) {//获取树控件路径
	CString strRet, strTmp;
	do {
		strTmp = m_tree.GetItemText(hTree);//获取当前树控件的文本
		strRet = strTmp + "\\" + strRet;
		hTree = m_tree.GetParentItem(hTree);//检索树项的父类句柄
	} while (hTree != NULL);
	return strRet;
}

void CRemoteClientDlg::DeleteTreeChild(HTREEITEM hTreeSelected)
{
	HTREEITEM hChi = NULL;
	HTREEITEM hNex = NULL;
	hChi = m_tree.GetChildItem(hTreeSelected);//检索树视图子集
	do {
		hNex = m_tree.GetNextItem(hChi, TVGN_NEXT);//检索下一个同级项
		if (hChi != NULL)
			m_tree.DeleteItem(hChi);//删除树视图控件某一项
		hChi = hNex;//指向同级下一个
	} while (hChi != NULL);
}

void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);//检索鼠标光标的屏幕坐标
	m_tree.ScreenToClient(&ptMouse);//将指定屏幕坐标转换为客户端坐标
	HTREEITEM hTreeSelected = m_tree.HitTest(ptMouse, 0);//HTREEITEM 是CTreeCtrl控件的项句柄，用于给树控件添加、查询、删除操作   HitTest()确定指定点相对于树视图工作区的相对位置
	if (hTreeSelected == NULL)
		return;
	if (m_tree.GetChildItem(hTreeSelected) == NULL) {
		//TODO: 如果是文件双击处理
		return;
	}
	DeleteTreeChild(hTreeSelected);//删除树控键子项
	m_List.DeleteAllItems();//删除列表控件所有项
	CString strPath = GetPath(hTreeSelected);//获取树控件路径
	SendPacket(2, (BYTE*)(LPCSTR)strPath, strPath.GetLength(), false);
	PFILEINFO tempfile = (PFILEINFO)m_hSocket->Getpacket().strData.c_str();
	int count = 0;
	while (tempfile->IsHasNext) {//判断文件是否有后续
		TRACE("[%s] IsDirectory:%d\r\n", tempfile->szFileName, tempfile->IsDirectory);
		if (tempfile->IsDirectory) {//目录
			if (CString(tempfile->szFileName) == "." || CString(tempfile->szFileName) == "..") {
				int ret = m_hSocket->Recv();
				if (ret != 2)break;
				tempfile = (PFILEINFO)m_hSocket->Getpacket().strData.c_str();
				continue;
			}
			else {//正常的目录
				HTREEITEM hTemp = m_tree.InsertItem(tempfile->szFileName, hTreeSelected, TVI_LAST);//树视图控件插入新项，后俩是插入项父级句柄和新项句柄。 插入成功返回新项的句柄
				m_tree.InsertItem(NULL, hTemp, TVI_LAST);
			}
		}
		else {//文件
			m_List.InsertItem(0, tempfile->szFileName);//在列表视图控件中插入新项。第一个参数0：在列表第一行插入一行；-1：在列表末尾插入一行
		}
		int ret = m_hSocket->Recv();
		if (ret != 2)break;
		tempfile = (PFILEINFO)m_hSocket->Getpacket().strData.c_str();//可以从const char* 强制转换为结构体的指针FILEINFO*.
		count++;
	}
	TRACE("%s路径下有%d个文件！\n", strPath, count);
	m_hSocket->CloseSocket();
}

void CRemoteClientDlg::LoadFileCurrent()
{
	HTREEITEM hTreeSelected = m_tree.GetSelectedItem();
	if (hTreeSelected == NULL)
		return;
	if (m_tree.GetChildItem(hTreeSelected) == NULL) {
		//TODO: 如果是文件双击处理
		return;
	}
	DeleteTreeChild(hTreeSelected);//删除树控键子项
	m_List.DeleteAllItems();//删除列表控件所有项
	CString strPath = GetPath(hTreeSelected);//获取树控件路径
	SendPacket(2, (BYTE*)(LPCSTR)strPath, strPath.GetLength(), false);
	PFILEINFO tempfile = (PFILEINFO)m_hSocket->Getpacket().strData.c_str();
	int count = 0;
	while (tempfile->IsHasNext) {//判断文件是否有后续
		TRACE("[%s] IsDirectory:%d\r\n", tempfile->szFileName, tempfile->IsDirectory);
		if (tempfile->IsDirectory) {//目录
			if (CString(tempfile->szFileName) == "." || CString(tempfile->szFileName) == "..") {
				int ret = m_hSocket->Recv();
				if (ret != 2)break;
				tempfile = (PFILEINFO)m_hSocket->Getpacket().strData.c_str();
				continue;
			}
			else {//正常的目录
				HTREEITEM hTemp = m_tree.InsertItem(tempfile->szFileName, hTreeSelected, TVI_LAST);//树视图控件插入新项，后俩是插入项父级句柄和新项句柄。 插入成功返回新项的句柄
				m_tree.InsertItem(NULL, hTemp, TVI_LAST);
			}
		}
		else {//文件
			m_List.InsertItem(0, tempfile->szFileName);//在列表视图控件中插入新项。第一个参数0：在列表第一行插入一行；-1：在列表末尾插入一行
		}
		int ret = m_hSocket->Recv();
		if (ret != 2)break;
		tempfile = (PFILEINFO)m_hSocket->Getpacket().strData.c_str();//可以从const char* 强制转换为结构体的指针FILEINFO*.
		count++;
	}
	TRACE("%s路径下有%d个文件！\n", strPath, count);
	m_hSocket->CloseSocket();
}

unsigned __stdcall CRemoteClientDlg::threadEntryForDownFile(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadDownFile();
	_endthreadex(0);//终止线程
	return 0;
}

void CRemoteClientDlg::threadDownFile()//不能只将一部分放在线程里，外部还有循环
{
	int nListSelected = m_List.GetSelectionMark();//检索列表视图控件的选择标记,返回标记的记号，从0开始
	CString ListText = m_List.GetItemText(nListSelected, 0);//定位到列表视图的行和列，从0开始

	//先选择下载路径，在开始下载传输
	CFileDialog dlg(FALSE, "*", ListText.GetString(), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, NULL, this);//构造标准Windows文件对话框，倒数第二项文件筛选器为空，因为可以下载任意文件，不需要筛选
	if (IDOK == dlg.DoModal()) {//显示文件对话框
		//要在发送传输之前创建文件
		m_status.m_info.SetWindowText("DownLoading...");
		//m_status.SetActiveWindow();//激活窗口
		m_status.CenterWindow(this);//使窗口相对于其父级居中
		m_status.ShowWindow(SW_SHOW);
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");//因为使用二进制方式读取的，文本可以通过二进制方式写入，dlg.GetPathName()文件的完整路径
		if (pFile == NULL) {
			AfxMessageBox("文件创建失败");
			m_status.ShowWindow(SW_HIDE);//下载失败要将对话框销毁掉
			EndWaitCursor();//从沙漏光标返回上一个光标
			return;
		}
		HTREEITEM hSelected = m_tree.GetSelectedItem();//检索树视图控件的当前选定项
		CString FileDown = GetPath(hSelected) + ListText;
		TRACE("FileDown=%s\n", (LPCSTR)FileDown);
		do {//该循环只会执行一次，方便出错直接退出循环
			//int ret = SendPacket(4, (BYTE*)(LPCSTR)FileDown, FileDown.GetLength(), FALSE);//SendPacket这个函数是外部的线程不能随便用
			//int ret = OnSendPacket(4 << 1 | 0, (LPARAM)(LPCSTR)FileDown);
			int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)FileDown);//将制定消息发送到一个或多个窗口
			if (ret < 0) {
				AfxMessageBox("下载失败");
				break;
			}
			//文件大小
			long long data = *(long long*)m_hSocket->Getpacket().strData.c_str();//将字符串转换成long long整型
			if (data == 0) {
				AfxMessageBox("文件长度为0，无法读取文件!");
				break;
			}

			long long nCount = 0;
			while (nCount < data) {
				int ret = m_hSocket->Recv();
				if (ret < 0) {
					AfxMessageBox("传输失败");
					break;
				}
				size_t len = fwrite(m_hSocket->Getpacket().strData.c_str(), 1, m_hSocket->Getpacket().strData.size(), pFile);
				nCount += len;
			}
			TRACE("接收到文件大小:%d\n", nCount);
			if (nCount == data)
				AfxMessageBox("下载成功!", MB_SETFOREGROUND);
		} while (false);
		fclose(pFile);
		m_hSocket->CloseSocket();
	}
	m_status.ShowWindow(SW_HIDE);
	EndWaitCursor();//从沙漏光标返回上一个光标
}

unsigned __stdcall CRemoteClientDlg::threadEntryWatchData(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadWatchData();
	_endthreadex(0);//终止线程
	return 0;
}

void CRemoteClientDlg::threadWatchData()
{
	Sleep(50);
	//ULONGLONG ret = GetTickCount64();//检索自启动以来经过的毫秒数
	for (;;) {//等价于while(true)
		//if (GetTickCount64() - ret < 50) { //这里是每过50ms在接收数据
		//	Sleep(50 + ret - GetTickCount64());
		//	//Sleep(GetTickCount64() - ret);
		//	ret = GetTickCount64();
		//}
		
		//int ret = SendPacket(6, NULL, 0);
		if (m_isFull == false) {//将判断是否有缓存放到接受数据之前，防止有缓存未处理接收下一数据，导致等待时间过长出现卡顿。
			BYTE* Data = NULL;
			int ret = SendMessage(WM_SEND_PACKET, 6 << 1 | 1, (LPARAM)Data);
			if (ret == 6) {//更新数据到缓存器
				
				BYTE* pData = (BYTE*)m_hSocket->Getpacket().strData.c_str();
				//存入CImage
				IStream* pStream = NULL;//创建一个流
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//从堆中分配指定的字节数，GMEM_MOVEABLE分配可移动内存
				if (hMem == NULL) {
					TRACE("分配移动内存失败errno=%d\n", GetLastError());
					Sleep(1);//要等待一下，防止出现死循环
					continue;
				}
				HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//创建一个流对象，用HGLOBAL句柄来存储流内容
				if (ret == S_OK) {//函数执行成功
					ULONG length = 0;
					pStream->Write(pData, m_hSocket->Getpacket().strData.size(), &length);//数据从缓存区中写入指定的流。
					LARGE_INTEGER bg = { 0 };
					pStream->Seek(bg, STREAM_SEEK_SET, NULL);//查找指针更改为新位置
					m_image.Load(pStream);//加载图像
					m_isFull = true;
				}
			}
			else
				Sleep(1);
		}//如果还有缓存，就啥也不干，等待无缓存再接受数据，就不需要等待50ms了
		else
			Sleep(1);
	}
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)//树形控件左键双击事件
{
	// TODO: 在此添加控件通知处理程序代码
	LoadFileInfo();
	*pResult = 0;
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)//树形控件左键单击事件
{
	// TODO: 在此添加控件通知处理程序代码
	//LoadFileInfo();
	*pResult = 0;
}

void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)//列表视图控件右键单击事件
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);//检索鼠标屏幕坐标
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int ListSelected = m_List.HitTest(ptList);//确定为于指定位置的列表视图项(如果有),返回列表的序号
	if (ListSelected < 0)
		return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);//加载菜单资源
	CMenu* pPup = menu.GetSubMenu(0);//检索弹出的菜单对象;第一个菜单项的位置值从0开始。
	if (pPup != NULL) {
		pPup->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, ptMouse.x, ptMouse.y, this);//在指定位置上显示浮动的弹出菜单，并跟踪菜单上项的选择情况
	}
	*pResult = 0;
}

void CRemoteClientDlg::OnDownloadFile()
{
	//添加线程函数处理大文件的下载
	unsigned thraddr;
	BeginWaitCursor();//将光标显示为沙漏光标，向用户显示正处于繁忙状态，通常表示等待，计算机正在处理数据
	_beginthreadex(NULL, 0, CRemoteClientDlg::threadEntryForDownFile, this, 0, &thraddr);//这里threadDownFile 线程函数要定义为静态的，否则访问不了

	Sleep(50);//等待50ms开启一个线程
	//TODO: 大文件传输需要额外处理

}


void CRemoteClientDlg::OnDeleteFile()//删除文件
{
	// TODO: 在此添加命令处理程序代码
	int ListSelected = m_List.GetSelectionMark();
	CString ListTect = m_List.GetItemText(ListSelected, 0);
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	CString FileDel = GetPath(hSelected) + ListTect;
	SendPacket(7, (BYTE*)(LPCSTR)FileDel, FileDel.GetLength());
	AfxMessageBox(m_hSocket->Getpacket().strData.c_str());
	LoadFileCurrent();//刷新一下目录下的文件
}


void CRemoteClientDlg::OnOpenFile()//打开文件
{
	// TODO: 在此添加命令处理程序代码
	int ListSelected = m_List.GetSelectionMark();
	CString ListTect = m_List.GetItemText(ListSelected, 0);
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	CString FileOpen = GetPath(hSelected) + ListTect;
	SendPacket(3, (BYTE*)(LPCSTR)FileOpen, FileOpen.GetLength());
}

LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)// WPARAM 和 LPARAM 各占4字节
{
	CString strPath = (LPCSTR)lParam;//LPCSTR 用于传递指向以NULL字符结尾的常量字符串的指针参数；LPCTSTR 和LPCSTR 是在Unicode字符集(宽字符集)环境下传递宽字符字符串参数。
	int ret = SendPacket(wParam >> 1, (BYTE*)(LPCSTR)strPath, strPath.GetLength(), wParam & 1);
	return ret;
}


void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	// TODO: 在此添加控件通知处理程序代码
	unsigned thraddr;
	CWatchDialog dlg;
	_beginthreadex(NULL, 0, CRemoteClientDlg::threadEntryWatchData, this, 0, &thraddr);
	//此时因为监控对话框定义的是模态的，所以不用担心狂点远程监控按钮的问题
	//GetDlgItem(IDC_BTN_START_WATCH)->EnableWindow(FALSE);//EnableWindow 启用或禁用鼠标和键盘输入，TRUE:启用 FALSE:禁用 防止狂点

	dlg.DoModal();//将监控对话框设为模态对话框
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}
