// LockDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "afxdialogex.h"
#include "LockDialog.h"


// CLockDialog 对话框

IMPLEMENT_DYNAMIC(CLockDialog, CDialog)

CLockDialog::CLockDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_LOCK, pParent)
{

}

CLockDialog::~CLockDialog()
{
}

void CLockDialog::DoDataExchange(CDataExchange* pDX)//和对话框交换验证数据
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLockDialog, CDialog)
END_MESSAGE_MAP()


// CLockDialog 消息处理程序
