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

BOOL CTool::Byte2Image(std::string& pData, CImage& m_image)
{
	IStream* pStream = NULL;//����һ����
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//�Ӷ��з���ָ�����ֽ�����GMEM_MOVEABLE������ƶ��ڴ�
	if (hMem == NULL) {
		TRACE("�����ƶ��ڴ�ʧ��errno=%d\n", GetLastError());
		Sleep(1);//Ҫ�ȴ�һ�£���ֹ������ѭ��
		return FALSE;
	}
	HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);//����һ����������HGLOBAL������洢������
	if (ret == S_OK) {//����ִ�гɹ�
		ULONG length = 0;
		pStream->Write(pData.c_str(), pData.size(), &length);//���ݴӻ�������д��ָ��������
		LARGE_INTEGER bg = { 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);//����ָ�����Ϊ��λ��
		if (m_image != NULL)
			m_image.Destroy();
		m_image.Load(pStream);//����ͼ��
	}
	return TRUE;
}
