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
