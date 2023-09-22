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
	//打包数据
	CPacket(WORD nCmd, const char* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;//+命令和校验是包长度
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
	CPacket(const BYTE* pData, size_t nSize) {//方便解析数据，随便丢进来一个数据进行解析  BYTE:unsigned char BYTE 1字节
		size_t i = 0;//字节长度跟踪
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {//找到一个包头
				sHead = *(WORD*)(pData + i);
				i += 2;//跳过包头
				break;
			}
		}
		if ((i + 4 + 2 + 2) > nSize) {//未找到包头或包头后无数据;i+4+2+2是除去包长度、命令、和校验后判断是否还有数据，如果没有直接退出函数
			nSize = 0;//将nSize清0
			return;
		}
		nLength = *(DWORD*)(pData + i);//获取包长度
		i += 4;
		if ((nLength + i) > nSize) {//数据没收全，只收到一半，说明缓冲区太小，数据包没接收完整，退出函数；因为包长度是从命令开始，到校验结束，所以这时开始判断。
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);//获取命令
		i += 2;
		//获取包数据
		if (nLength > 4) {//包长度>4才会有包数据的位置
			strData.resize(nLength - 2 - 2);//为字符串指定新的大小；设置包数据的大小
			memcpy((void*)strData.c_str(), (pData + i), (nLength - 2 - 2));//c_str()返回当前字符串的首字符地址，指向以空字符终止的数组；data()与c_str()类似，但返回的数组不以空字符终止
			i += (nLength - 4);
		}
		sSum = *(WORD*)(pData + i);//获取和校验
		i += 2;
		//校验一下
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {//.size()返回字符串中元素的当前数目
			sum += ((BYTE)strData[j] & 0xFF);//保持二进制补码的一致性
		}
		if (sum == sSum) {//解析成功
			nSize = i;//等于整个数据包的长度，这里使用i而不是数据包长度，是因为前面还有废数据
			return;
		}
		nSize = 0;//解析失败
	}
	~CPacket() {}

	CPacket operator=(const CPacket& pack) {//运算符重载
		if (this != &pack) {//this指向类自身的变量地址
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size() {//数据包的长度
		return nLength + 2 + 4;
	}

	const char* Data() {//将整个数据包的数据转成字符串型，方便查看读取。
		strOut.resize(Size());//将strOut字符串大小指定为整个数据包的长度
		BYTE* pData = (BYTE*)strOut.c_str();//定义一个BYTE数据类型的指针指向整个包的数据字符串
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), nLength - 4); pData += strData.size();
		*(WORD*)pData = sSum; pData += 2;
		return strOut.c_str();
	}

public://包数据是外部需要调用到的，所以这里用public
	WORD sHead;//包头FE FF  unsigned short 2字节
	DWORD nLength;//包长度，数据的字节长度(从命令开始，到和校验结束)  unsigned long 4字节
	WORD sCmd;//命令
	std::string strData;//包数据
	WORD sSum;//和校验 将包数据进行加求和
	std::string strOut;//整个包的数据，方便查看0001000000000001
};
#pragma pack(pop)
