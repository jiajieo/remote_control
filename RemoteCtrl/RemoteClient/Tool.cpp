#include "pch.h"
#include "Tool.h"

void CTool::Dump(BYTE* pData, size_t nSize)//查看输出打包好的数据
{
	std::string strOut;
	char buf[10] = "";
	for (size_t i = 0; i < nSize; i++) {
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);//%02X:X表示以16进制输出，02表示以两位输出
		strOut += buf;
		if ((i + 1) % 16 == 0)strOut += "\n";
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());//将字符串发送到调试器进行显示
	//结果发现数据不对：FFFECCCC090000000100CCCCD8EF41，然后首先将CPacket类的字节进行对齐操作
	//TRACE("%s\r\n", strOut.c_str());
}

BOOL CTool::Byte2Image(std::string& pData, CImage& m_image)
{
	IStream* pStream = NULL;//创建一个流
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//从堆中分配指定的字节数，GMEM_MOVEABLE分配可移动内存
	if (hMem == NULL) {
		TRACE("分配移动内存失败errno=%d\n", GetLastError());
		Sleep(1);//要等待一下，防止出现死循环
		return FALSE;
	}
	HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//创建一个流对象，用HGLOBAL句柄来存储流内容
	if (ret == S_OK) {//函数执行成功
		ULONG length = 0;
		pStream->Write(pData.c_str(), pData.size(), &length);//数据从缓存区中写入指定的流。
		LARGE_INTEGER bg = { 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);//查找指针更改为新位置
		if (m_image != NULL)
			m_image.Destroy();
		m_image.Load(pStream);//加载图像
	}
	return TRUE;
}
