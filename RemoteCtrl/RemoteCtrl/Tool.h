#pragma once
class CTool
{
public:
	static void Dump(BYTE* pData, size_t nSize);
};

typedef struct fileinfo {//�ṹ��Ĭ����public,��Ĭ����private.
	fileinfo() {
		IsInvalid = FALSE;//Ĭ������Ч��
		IsDirectory = -1;
		IsHasNext = TRUE;//Ĭ�����к�����
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//�жϸ�Ŀ¼�Ƿ���Ч
	BOOL IsDirectory;//�ж���Ŀ¼(�ļ���)�����ļ� 0:�ļ� 1:Ŀ¼
	BOOL IsHasNext;//�ļ��Ƿ��к��� 1:�� 0:��
	char szFileName[256];//�ļ���
}FILEINFO, * PFILEINFO;


typedef struct mouseev {
	mouseev() {//��ʼ��
		nAction = 0;
		nButton = 0;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//����������������:���(������˫��)���ƶ�
	WORD nButton;//������Ҽ����м�
	POINT ptXY;//����
}MOUSEEV, * PMOUSEEV;