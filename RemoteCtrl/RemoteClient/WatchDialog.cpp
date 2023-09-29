// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "ClientControler.h"


// CWatchDialog 对话框
IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_width = -1;
	m_height = -1;
	islock = false;
	m_isFull = false;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
	DDX_Control(pDX, IDC_BTN_LOCK, m_lock);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MBUTTONDBLCLK()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO:  在此添加额外的初始化
	SetTimer(0, 45, NULL);//安装系统计时器，第一个参数为0是该窗口的默认定时器，第二个参数指定时间间隔50ms
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0) {
		if (m_isFull == true) {//如果有缓存
			//TODO: 显示图片
			CRect rect;
			m_picture.GetWindowRect(rect);//获取对话框屏幕坐标
			CClientControler::getInstance()->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);//将位图从源设备上下文复制到当前设备上下文
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(),0,0,SRCCOPY);
			//BitBlt()将位图从源设备上下文复制到当前设备上下文；GetDC()检索指向工作区的设备上下文；GetSafeHdc()获取设备上下文的句柄m_hDC
			//m_picture.InvalidateRect(NULL);//重绘，将给定矩形添加到更新区域 这里不需要

			//远程端屏幕大小
			if (m_width == -1 || m_height == -1) {
				m_width = CClientControler::getInstance()->GetImage().GetWidth();
				m_height = CClientControler::getInstance()->GetImage().GetHeight();
			}

			CClientControler::getInstance()->GetImage().Destroy();//分离位图并销毁位图
			SetNoImage();//设为无缓存 m_isFull=false
		}
	}

	CDialog::OnTimer(nIDEvent);
}


CPoint CWatchDialog::ConvertRemoteScreenPoint(CPoint& point, bool IsClientCoor)//将控制端展现的鼠标坐标转换为远程端被控画面的坐标
{
	if (m_width != -1 && m_height != -1) {
		CRect Clientrect;
		//加个条件:如果坐标已经在客户端坐标内就不用再转换为客户端坐标了
		if (IsClientCoor)
			ScreenToClient(&point);//将给定的坐标转换为客户端坐标
		m_picture.GetWindowRect(&Clientrect);//获取客户端屏幕大小
		int width0 = Clientrect.Width();
		int height0 = Clientrect.Height();

		//远程端的鼠标坐标
		int x = point.x * m_width / width0;
		int y = (point.y-33) * m_height / height0;
		return CPoint(x, y);
	}
	return 0;
}
//typedef struct mouseev {
//	mouseev() {//初始化
//		nAction = 0;
//		nButton = 0;
//		ptXY.x = 0;
//		ptXY.y = 0;
//	}
//	WORD nAction;//首先是描述动作的:单击0、双击1、按下2、放开4
//	WORD nButton;//左键0、右键1、中键2、没有按键3
//	POINT ptXY;//坐标
//}MOUSEEV, * PMOUSEEV;

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//左键双击
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 1;
	mouse.nButton = 0;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	//左键按下

	CPoint remote = ConvertRemoteScreenPoint(point);
	TRACE("本地x=%d,y=%d\n", point.x, point.y);
	TRACE("远程x=%d,y=%d\n", remote.x, remote.y);
	MOUSEEV mouse;
	mouse.nAction = 2;
	mouse.nButton = 0;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	//右键双击
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 1;
	mouse.nButton = 1;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	//右键按下
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 2;
	mouse.nButton = 1;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	//左键放开
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 4;
	mouse.nButton = 0;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	//右键放开
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 4;
	mouse.nButton = 1;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	//鼠标移动
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 5;
	mouse.nButton = 3;
	mouse.ptXY = remote;
	//TODO:使用SendMessage发送消息时存在一个设计隐患，网络通信和对话框有耦合
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE,&plstPack);
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnMButtonDown(UINT nFlags, CPoint point)
{
	//中键按下
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 2;
	mouse.nButton = 2;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnMButtonDown(nFlags, point);
}


void CWatchDialog::OnMButtonUp(UINT nFlags, CPoint point)
{
	//中键放开
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 4;
	mouse.nButton = 2;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnMButtonUp(nFlags, point);
}


void CWatchDialog::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	//中键双击
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 1;
	mouse.nButton = 2;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
	CDialog::OnMButtonDblClk(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()//只有将Picture Control（图片控件）的属性：通知设置为true才会响应SS_NOTIFY样式的静态控件。但是这样就无法接收到按下、放开、双击等消息了，所以这里我们不用SS_NOTIFY样式的静态控件。
{
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);
	CPoint remote = ConvertRemoteScreenPoint(point, true);
	MOUSEEV mouse;
	mouse.nAction = 0;
	mouse.nButton = 0;
	mouse.ptXY = remote;
	std::list<CPacket> plstPack;
	CClientControler::getInstance()->SendPacket(5, (BYTE*)&mouse, sizeof(mouse), TRUE, &plstPack);
}


void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	// TODO: 在此添加控件通知处理程序代码
	if (islock==false) {//没锁，点的是锁机
		m_lock.SetWindowText("解锁");
		islock = true;
		std::list<CPacket> plstPack;
		CClientControler::getInstance()->SendPacket(8, NULL, 0, TRUE, &plstPack);
	}
	else if (islock=true) {//锁了，点的是解锁
		m_lock.SetWindowText("锁机");
		islock = false;
		std::list<CPacket> plstPack;
		CClientControler::getInstance()->SendPacket(9, NULL, 0, TRUE, &plstPack);
	}
}
