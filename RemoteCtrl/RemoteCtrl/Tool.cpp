#include "pch.h"
#include "Tool.h"

void CTool::Dump(BYTE* pData, size_t nSize)//�鿴�������õ�����
{
	std::string strOut;
	char buf[10] = "";
	for (size_t i = 0; i < nSize; i++) {
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);//%02X:X��ʾ��16���������02��ʾ����λ���
		strOut += buf;
		if ((i + 1) % 16 == 0)strOut += "\n";
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());//���ַ������͵�������������ʾ
	//����������ݲ��ԣ�FFFECCCC090000000100CCCCD8EF41��Ȼ�����Ƚ�CPacket����ֽڽ��ж������
	//TRACE("%s\r\n", strOut.c_str());
}
