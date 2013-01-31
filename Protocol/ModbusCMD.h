#pragma once
#include <vector>
#include "../PublicSupport/Dat2cTypeDef.h"

namespace Protocol {

const char YxDataType = 1;
const char YcDataType = 2;
const char YmDataType = 3;
const char YkDataType = 4;
const char SynTimeType = 5;

struct stModbusCmd
{
	unsigned short FunCode_;            //报文功能码
	unsigned short RegisterAddr_;      //要操作的寄存器地址
	unsigned short DataNum_;           //要操作的数据数量

	unsigned char PointDataType_;      //对应数据存储实例的数据类型
	size_t PointIndex_;                //对应数据存储实例的数据点号

	stModbusCmd(unsigned char funCode,unsigned short registerAddr,unsigned short DataNum,unsigned char PointDataType,size_t PointIndex)
		:FunCode_(funCode),
		RegisterAddr_(registerAddr),
		DataNum_(DataNum),
		PointDataType_(PointDataType),
		PointIndex_(PointIndex)
	{

	}

	stModbusCmd()
	{
		FunCode_ = 0;
		RegisterAddr_ = 0;
		DataNum_ = 0;
		PointDataType_ = 0;
		PointIndex_ = 0;
	}
};

class CModbusCmdPara
{
public:
	CModbusCmdPara(typeAddr addr);
	CModbusCmdPara(const CModbusCmdPara & rhs);
	virtual ~CModbusCmdPara(void);
	CModbusCmdPara & operator = (const CModbusCmdPara & rhs);

	//bool getbInit();
	typeAddr getAddr();
	int getNextReadCmd(stModbusCmd & val);
	stModbusCmd getReadCMD(int index);
	stModbusCmd getWriteCMD(int index);
	int getWriteCmd(unsigned char PointDataType,size_t PointIndex,stModbusCmd & val);
	int AddReadCmd(stModbusCmd val);
	int AddWriteCmd(stModbusCmd val);
	int DelReadCmd(int index);
	int DelWriteCmd(int index);
	int getReadCmdSum();
	int getWriteCmdSum();
	static std::string TransDataTypeToString(char val);
	static int TransDataTypeFromString(std::string val);
	
private:
	int InitDefaultPara();

private:
	int ReadIndex_;                        //接收命令容器的遍历指针
	typeAddr addr_;                  //该命令参数对应的通讯结点的地址
	//bool bInitialized_;                    //该命令参数是否被初始化过
	std::vector<stModbusCmd> ReadCmdSet_;  //接收命令容器
	std::vector<stModbusCmd> WriteCmdSet_; //发送命令容器
};

};//namespace Protocol 

