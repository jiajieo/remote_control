#pragma once
#include <wtypes.h>
#include <string>

#pragma pack(push)
#pragma pack(1)
class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {

	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	//�������
	CPacket(WORD nCmd, const char* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;//+�����У���ǰ�����
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((char*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t i = 0; i < strData.size(); i++) {
			sSum += ((BYTE)strData[i] & 0xFF);
		}
	}
	CPacket(const BYTE* pData, size_t nSize) {//����������ݣ���㶪����һ�����ݽ��н���  BYTE:unsigned char BYTE 1�ֽ�
		size_t i = 0;//�ֽڳ��ȸ���
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {//�ҵ�һ����ͷ
				sHead = *(WORD*)(pData + i);
				i += 2;//������ͷ
				break;
			}
		}
		if ((i + 4 + 2 + 2) > nSize) {//δ�ҵ���ͷ���ͷ��������;i+4+2+2�ǳ�ȥ�����ȡ������У����ж��Ƿ������ݣ����û��ֱ���˳�����
			nSize = 0;//��nSize��0
			return;
		}
		nLength = *(DWORD*)(pData + i);//��ȡ������
		i += 4;
		if ((nLength + i) > nSize) {//����û��ȫ��ֻ�յ�һ�룬˵��������̫С�����ݰ�û�����������˳���������Ϊ�������Ǵ����ʼ����У�������������ʱ��ʼ�жϡ�
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);//��ȡ����
		i += 2;
		//��ȡ������
		if (nLength > 4) {//������>4�Ż��а����ݵ�λ��
			strData.resize(nLength - 2 - 2);//Ϊ�ַ���ָ���µĴ�С�����ð����ݵĴ�С
			memcpy((void*)strData.c_str(), (pData + i), (nLength - 2 - 2));//c_str()���ص�ǰ�ַ��������ַ���ַ��ָ���Կ��ַ���ֹ�����飻data()��c_str()���ƣ������ص����鲻�Կ��ַ���ֹ
			i += (nLength - 4);
		}
		sSum = *(WORD*)(pData + i);//��ȡ��У��
		i += 2;
		//У��һ��
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {//.size()�����ַ�����Ԫ�صĵ�ǰ��Ŀ
			sum += ((BYTE)strData[j] & 0xFF);//���ֶ����Ʋ����һ����
		}
		if (sum == sSum) {//�����ɹ�
			nSize = i;//�����������ݰ��ĳ��ȣ�����ʹ��i���������ݰ����ȣ�����Ϊǰ�滹�з�����
			return;
		}
		nSize = 0;//����ʧ��
	}
	~CPacket() {}

	CPacket operator=(const CPacket& pack) {//���������
		if (this != &pack) {//thisָ��������ı�����ַ
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size() {//���ݰ��ĳ���
		return nLength + 2 + 4;
	}

	const char* Data() {//���������ݰ�������ת���ַ����ͣ�����鿴��ȡ��
		strOut.resize(Size());//��strOut�ַ�����Сָ��Ϊ�������ݰ��ĳ���
		BYTE* pData = (BYTE*)strOut.c_str();//����һ��BYTE�������͵�ָ��ָ���������������ַ���
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), nLength - 4); pData += strData.size();
		*(WORD*)pData = sSum; pData += 2;
		return strOut.c_str();
	}

public://���������ⲿ��Ҫ���õ��ģ�����������public
	WORD sHead;//��ͷFE FF  unsigned short 2�ֽ�
	DWORD nLength;//�����ȣ����ݵ��ֽڳ���(�����ʼ������У�����)  unsigned long 4�ֽ�
	WORD sCmd;//����
	std::string strData;//������
	WORD sSum;//��У�� �������ݽ��м����
	std::string strOut;//�����������ݣ�����鿴0001000000000001
};
#pragma pack(pop)
