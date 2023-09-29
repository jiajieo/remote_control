﻿
// RemoteClientDlg.cpp: 实现文件
//


#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"
#include "ClientControler.h"



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
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS, &CRemoteClientDlg::OnIpnFieldchangedIpaddress)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CRemoteClientDlg::OnEnChangeEditPort)
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
	m_servaddress = 0x7F000001;//127.0.0.1->192.168.148.128
	m_port = "6000";
	int port = atoi(m_port);
	CClientControler::getInstance()->UpdataAddress(m_servaddress, port);
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
	if (SendPacket(1981).sCmd == 1981)
		MessageBox("连接成功");
}


void CRemoteClientDlg::OnBnClickedBtnViewfile()//查看文件
{
	// TODO: 在此添加控件通知处理程序代码

	std::string drivers = SendPacket(1).strData;
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
	m_tree.InsertItem(NULL, hTemp, TVI_LAST);//使该磁盘有后续
}

CPacket CRemoteClientDlg::SendPacket(WORD nCmd, BYTE* pData, size_t nSize, BOOL bAutoClose)//发送命令 
{
	std::list<CPacket> lstPack;
	CClientControler::getInstance()->SendPacket(nCmd, pData, nSize, bAutoClose, &lstPack);
	return lstPack.front();
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
	//PFILEINFO tempfile = (PFILEINFO)SendPacket(2, (BYTE*)(LPCSTR)strPath, strPath.GetLength(), false).strData.c_str();
	//CPacket pack(2, strPath.GetBuffer(), strPath.GetLength());
	std::list<CPacket> lstPack;
	CClientControler::getInstance()->SendPacket(2, (BYTE*)strPath.GetBuffer(), strPath.GetLength(), FALSE, &lstPack);
	if (lstPack.size() > 0) {
		int Count = 0;
		std::list<CPacket>::iterator it = lstPack.begin();
		for (; it != lstPack.end(); it++) {
			PFILEINFO tempfile = (PFILEINFO)it->strData.c_str();
			if (tempfile->IsHasNext) {
				if (tempfile->IsDirectory) {//目录
					if (CString(tempfile->szFileName) == "." || CString(tempfile->szFileName) == "..") {
						continue;
					}
					else {
						HTREEITEM hTemp = m_tree.InsertItem(tempfile->szFileName, hTreeSelected, TVI_LAST);//树视图控件插入新项，后俩是插入项父级句柄和新项句柄。 插入成功返回新项的句柄
						m_tree.InsertItem(NULL, hTemp, TVI_LAST);
					}
				}
				else {
					m_List.InsertItem(0, tempfile->szFileName);//在列表视图控件中插入新项。第一个参数0：在列表第一行插入一行；-1：在列表末尾插入一行
				}
				Count++;
			}
		}
		TRACE("%s路径下有%d个文件！\n", strPath, Count);
	}
}

void CRemoteClientDlg::LoadFileCurrent()
{
	HTREEITEM hTreeSelected = m_tree.GetSelectedItem();//检索当前选定项
	if (hTreeSelected == NULL)
		return;
	if (m_tree.GetChildItem(hTreeSelected) == NULL) {
		//TODO: 如果是文件双击处理
		return;
	}
	DeleteTreeChild(hTreeSelected);//删除树控键子项
	m_List.DeleteAllItems();//删除列表控件所有项
	CString strPath = GetPath(hTreeSelected);//获取树控件路径
	std::list<CPacket> lstPack;
	CClientControler::getInstance()->SendPacket(2, (BYTE*)(LPCSTR)strPath, strPath.GetLength(), false,&lstPack);
	if (lstPack.size() > 0) {
		int Count = 0;
		std::list<CPacket>::iterator it = lstPack.begin();
		for (; it != lstPack.end(); it++) {
			PFILEINFO tempfile = (PFILEINFO)it->strData.c_str();
			if (tempfile->IsHasNext) {
				if (tempfile->IsDirectory) {//目录
					if (CString(tempfile->szFileName) == "." || CString(tempfile->szFileName) == "..") {
						continue;
					}
					else {
						HTREEITEM hTemp = m_tree.InsertItem(tempfile->szFileName, hTreeSelected, TVI_LAST);//树视图控件插入新项，后俩是插入项父级句柄和新项句柄。 插入成功返回新项的句柄
						m_tree.InsertItem(NULL, hTemp, TVI_LAST);
					}
				}
				else {
					m_List.InsertItem(0, tempfile->szFileName);//在列表视图控件中插入新项。第一个参数0：在列表第一行插入一行；-1：在列表末尾插入一行
				}
				Count++;
			}
		}
		TRACE("%s路径下有%d个文件！\n", strPath, Count);
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
	int nListSelected = m_List.GetSelectionMark();//检索列表视图控件的选择标记,返回标记的记号，从0开始
	CString ListText = m_List.GetItemText(nListSelected, 0);//定位到列表视图的行和列，从0开始
	HTREEITEM hSelected = m_tree.GetSelectedItem();//检索树视图控件的当前选定项
	CString FileDown = GetPath(hSelected) + ListText;//下载路径

	//BeginWaitCursor();//将光标显示为沙漏光标，向用户显示正处于繁忙状态，通常表示等待，计算机正在处理数据
	CClientControler::getInstance()->DownLoadFile(ListText, FileDown);
	//EndWaitCursor();//从沙漏光标返回上一个光标
}


void CRemoteClientDlg::OnDeleteFile()//删除文件
{
	// TODO: 在此添加命令处理程序代码
	int ListSelected = m_List.GetSelectionMark();
	CString ListTect = m_List.GetItemText(ListSelected, 0);
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	CString FileDel = GetPath(hSelected) + ListTect;
	SendPacket(7, (BYTE*)(LPCSTR)FileDel, FileDel.GetLength());
	AfxMessageBox(CClientControler::getInstance()->Getpacket().strData.c_str());
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
	return SendPacket(wParam >> 1, (BYTE*)(LPCSTR)strPath, strPath.GetLength(), wParam & 1).sCmd;//0是0000 0000,1是0000 0001别搞错了
}


void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	CClientControler::getInstance()->StartWatch();
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}


void CRemoteClientDlg::OnIpnFieldchangedIpaddress(NMHDR* pNMHDR, LRESULT* pResult)//IP
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	int port = atoi(m_port);
	CClientControler::getInstance()->UpdataAddress(m_servaddress, port);
	*pResult = 0;
}


void CRemoteClientDlg::OnEnChangeEditPort()//端口
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData();//检索控件中的数据，把控件的值赋给成员变量；FALSE将成员变量的值赋给控件
	int port = atoi(m_port);//将字符串转换为整数
	CClientControler::getInstance()->UpdataAddress(m_servaddress, port);
}
