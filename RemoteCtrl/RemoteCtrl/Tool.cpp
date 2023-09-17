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
