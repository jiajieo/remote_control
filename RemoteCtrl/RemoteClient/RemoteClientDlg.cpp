
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
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CRemoteClientDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_VIEWFILE, &CRemoteClientDlg::OnBnClickedBtnViewfile)
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



void CRemoteClientDlg::OnBnClickedBtnConnect()
{
	// TODO: 在此添加控件通知处理程序代码

	SendPacket(1981);
	int ret = m_hSocket->Recv();
	if (ret == 1981)
		MessageBox("连接成功");
}


void CRemoteClientDlg::OnBnClickedBtnViewfile()//查看文件
{
	// TODO: 在此添加控件通知处理程序代码
	SendPacket(1);
	int ret = m_hSocket->Recv();
	std::string drivers=m_hSocket->Getpacket().strData;
	std::string dr;
	TRACE("drivers.c_str():%s\n",drivers.c_str());
	m_tree.DeleteAllItems();//删除树视图控件的所有项
	for (size_t i = 0; i < drivers.size(); i++) {
		if (drivers[i] == ',') {
			
			m_tree.InsertItem(dr.c_str(),TVI_ROOT,TVI_LAST);//在树视图控件中插入某个新项  TVI_ROOT表示树形视图的根节点，TVI_LAST表示树形视图中的最后一个节点
			dr.clear();//清除字符串元素
			continue;
		}
		dr += drivers[i];
		dr += ":";
	}
	m_tree.InsertItem(dr.c_str());
}

int CRemoteClientDlg::SendPacket(WORD nCmd, BYTE* pData, size_t nSize)
{
	UpdateData();//检索控件中的数据，把控件的值赋给成员变量；FALSE将成员变量的值赋给控件
	int port = atoi(m_port);//将字符串转换为整数

	m_hSocket = CClientSocket::getInstance();
	if (m_hSocket != NULL) {
		if (m_hSocket->InitSocket(m_servaddress, port) == true) {
			CPacket pack(nCmd, (const char*)pData, nSize);
			m_hSocket->Send(pack);
		}
	}

	return 0;
}
