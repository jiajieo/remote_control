#pragma once
class CTool
{
public:
	static void Dump(BYTE* pData, size_t nSize);
};

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


typedef struct mouseev {
	mouseev() {//初始化
		nAction = 0;
		nButton = 0;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//首先是描述动作的:点击(单击、双击)、移动
	WORD nButton;//左键、右键、中键
	POINT ptXY;//坐标
}MOUSEEV, * PMOUSEEV;