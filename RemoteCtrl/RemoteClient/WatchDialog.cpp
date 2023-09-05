// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"


// CWatchDialog 对话框
IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
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
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();//GetParent()获取父窗口的指针
		if (pParent->GetIsFull() == true) {//如果有缓存
			//TODO: 显示图片
			CRect rect;
			m_picture.GetWindowRect(rect);
			pParent->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(),0,0,rect.Width(),rect.Height(),SRCCOPY);//将位图从源设备上下文复制到当前设备上下文
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(),0,0,SRCCOPY);
			//BitBlt()将位图从源设备上下文复制到当前设备上下文；GetDC()检索指向工作区的设备上下文；GetSafeHdc()获取设备上下文的句柄m_hDC
			//m_picture.InvalidateRect(NULL);//重绘，将给定矩形添加到更新区域 这里不需要

			pParent->GetImage().Destroy();//分离位图并销毁位图
			pParent->SetNoImage();//设为无缓存 m_isFull=false
		}
	}

	CDialog::OnTimer(nIDEvent);
}


CPoint CWatchDialog::ConvertRemoteScreenPoint(CPoint& point)//将控制端展现的鼠标坐标转换为远程端被控画面的坐标
{
	CRect Clientrect;
	ScreenToClient(&point);//将给定的坐标转换为客户端坐标
	m_picture.GetWindowRect(&Clientrect);//获取客户端屏幕大小
	int width0=Clientrect.Width();
	int height0 = Clientrect.Height();
	//远程端屏幕大小
	int width = 1920;
	int height = 1080;
	//远程端的鼠标坐标
	int x = point.x * width / width0;
	int y = point.y * height / height0;
	return CPoint(x,y);
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	//左键按下
	CPoint remote = ConvertRemoteScreenPoint(point);
	MOUSEEV mouse;
	mouse.nAction = 2;
	mouse.nButton = 0;
	mouse.ptXY = remote;
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
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
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendPacket(5, (BYTE*)&mouse, sizeof(mouse));
	CDialog::OnMButtonDblClk(nFlags, point);
}
