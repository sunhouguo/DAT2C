#include <boost/algorithm/string/predicate.hpp>
#include "ModbusCMD.h"

namespace Protocol {

#define strYxDataType "YxDataType"
#define strYcDataType "YcDataType"
#define strYkDataType "YkDataType"
#define strYmDataType "YmDataType"
#define strSynTimeType "SynTimeType"

CModbusCmdPara::CModbusCmdPara(typeAddr addr)
							  :addr_(addr)
{
	//bInitialized_ = false;

	InitDefaultPara();
}

CModbusCmdPara::CModbusCmdPara(const CModbusCmdPara & rhs)
{
	addr_ = rhs.addr_;
	//bInitialized_ = rhs.bInitialized_;
	ReadIndex_ = rhs.ReadIndex_;
	ReadCmdSet_ = rhs.ReadCmdSet_;
	WriteCmdSet_ = rhs.WriteCmdSet_;
}

CModbusCmdPara::~CModbusCmdPara(void)
{
}

CModbusCmdPara & CModbusCmdPara::operator = (const CModbusCmdPara & rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	addr_ = rhs.addr_;
	//bInitialized_ = rhs.bInitialized_;
	ReadIndex_ = rhs.ReadIndex_;
	ReadCmdSet_ = rhs.ReadCmdSet_;
	WriteCmdSet_ = rhs.WriteCmdSet_;

	return *this;
}

//************************************
// Method:    InitDefaultPara
// FullName:  Protocol::CModbusCmdPara::InitDefaultPara
// Access:    private 
// Returns:   int
// Qualifier: 初始化自身参数
//************************************
int CModbusCmdPara::InitDefaultPara()
{
	ReadIndex_ = 0;

	return 0;
}

//************************************
// Method:    AddReadCmd
// FullName:  Protocol::CModbusCmdPara::AddReadCmd
// Access:    public 
// Returns:   int
// Qualifier: 添加一个读寄存器命令
// Parameter: stModbusCmd val：Modbus的命令数据结构
//************************************
int CModbusCmdPara::AddReadCmd(stModbusCmd val)
{
	ReadCmdSet_.push_back(val);

	//bInitialized_ = true;

	return 0;
}

//************************************
// Method:    AddWriteCmd
// FullName:  Protocol::CModbusCmdPara::AddWriteCmd
// Access:    public 
// Returns:   int
// Qualifier: 添加一个写寄存器命令
// Parameter: stModbusCmd val：Modbus的命令数据结构
//************************************
int CModbusCmdPara::AddWriteCmd(stModbusCmd val)
{
	WriteCmdSet_.push_back(val);

	//bInitialized_ = true;

	return 0;
}

//************************************
// Method:    DelReadCmd
// FullName:  Protocol::CModbusCmdPara::DelReadCmd
// Access:    public 
// Returns:   int：0删除成功；other出错
// Qualifier: 删除一个读寄存器命令
// Parameter: int index：命令在队列中的序号
//************************************
int CModbusCmdPara::DelReadCmd(int index)
{
	if (index < 0 || index >= (int)ReadCmdSet_.size())
	{
		return -1;
	}

	ReadCmdSet_.erase(ReadCmdSet_.begin() + index);

	return 0;
}

//************************************
// Method:    DelWriteCmd
// FullName:  Protocol::CModbusCmdPara::DelWriteCmd
// Access:    public 
// Returns:   int：0删除成功；other出错
// Qualifier: 删除一个写寄存器命令
// Parameter: int index：命令在队列中序号
//************************************
int CModbusCmdPara::DelWriteCmd(int index)
{
	if (index < 0 || index >= (int)WriteCmdSet_.size())
	{
		return -1;
	}

	WriteCmdSet_.erase(WriteCmdSet_.begin() + index);

	return 0;
}

//************************************
// Method:    getNextReadCmd
// FullName:  Protocol::CModbusCmdPara::getNextReadCmd
// Access:    public 
// Returns:   int：0执行成功；other无法获取指定命令
// Qualifier: 获得下一个读寄存器命令
// Parameter: Protocol::stModbusCmd & val：命令的输出参数
//************************************
int CModbusCmdPara::getNextReadCmd(Protocol::stModbusCmd &val)
{
	if (ReadCmdSet_.size() <= 0)
	{
		return -1;
	}

	val = ReadCmdSet_[ReadIndex_];

	ReadIndex_ = (ReadIndex_ + 1) % ReadCmdSet_.size();


	return 0;
}

//************************************
// Method:    getWriteCmd
// FullName:  Protocol::CModbusCmdPara::getWriteCmd
// Access:    public 
// Returns:   int：0执行成功；other无法获取指定命令
// Qualifier: 根据输入参数匹配写寄存器命令
// Parameter: unsigned char PointDataType：数据类型标识
// Parameter: size_t PointIndex：数据点号
// Parameter: stModbusCmd & val：命令的输出参数
//************************************
int CModbusCmdPara::getWriteCmd(unsigned char PointDataType,size_t PointIndex,stModbusCmd & val)
{
	for (int i=0;i<(int)WriteCmdSet_.size();i++)
	{
		if((WriteCmdSet_[i].PointDataType_ == PointDataType)&&(WriteCmdSet_[i].PointIndex_ == PointIndex))
		{
			val = WriteCmdSet_[i];

			return 0;
		}
	}

	return -1;
}

//************************************
// Method:    getReadCmdSum
// FullName:  Protocol::CModbusCmdPara::getReadCmdSum
// Access:    public 
// Returns:   int
// Qualifier: 返回读命令参数的数量
//************************************
int CModbusCmdPara::getReadCmdSum()
{
	return ReadCmdSet_.size();
}

//************************************
// Method:    getWriteCmdSum
// FullName:  Protocol::CModbusCmdPara::getWriteCmdSum
// Access:    public 
// Returns:   int
// Qualifier: 返回写命令参数的数量
//************************************
int CModbusCmdPara::getWriteCmdSum()
{
	return WriteCmdSet_.size();
}

//************************************
// Method:    getReadCMD
// FullName:  Protocol::CModbusCmdPara::getReadCMD
// Access:    public 
// Returns:   Protocol::stModbusCmd
// Qualifier: 按索引值获得一个读命令
// Parameter: int index
//************************************
stModbusCmd CModbusCmdPara::getReadCMD(int index)
{
	if (index < 0 || index >= (int)ReadCmdSet_.size())
	{
		return stModbusCmd();
	}

	return ReadCmdSet_[index];
}

//************************************
// Method:    getWriteCMD
// FullName:  Protocol::CModbusCmdPara::getWriteCMD
// Access:    public 
// Returns:   Protocol::stModbusCmd
// Qualifier: 按索引值获得一个写命令
// Parameter: int index
//************************************
stModbusCmd CModbusCmdPara::getWriteCMD(int index)
{
	if (index < 0 || index >= (int)WriteCmdSet_.size())
	{
		return stModbusCmd();
	}

	return WriteCmdSet_[index];
}

//************************************
// Method:    TransDataTypeToString
// FullName:  Protocol::CModbusCmdPara::TransDataTypeToString
// Access:    public 
// Returns:   std::string
// Qualifier: cmd命令的数据类型从程序宏到xml文件字符值的转换
// Parameter: char val
//************************************
std::string CModbusCmdPara::TransDataTypeToString(char val)
{
	std::string ret = "";

	switch (val)
	{
	case YxDataType:
		ret = strYxDataType;
		break;

	case YcDataType:
		ret = strYcDataType;
		break;

	case YkDataType:
		ret = strYkDataType;
		break;

	case YmDataType:
		ret = strYmDataType;
		break;

	case SynTimeType:
		ret = strSynTimeType;
		break;

	default:
		ret = "";
		break;
	}

	return ret;
}

//************************************
// Method:    TransDataTypeFromString
// FullName:  Protocol::CModbusCmdPara::TransDataTypeFromString
// Access:    public 
// Returns:   int
// Qualifier: cmd命令的数据类型从xml文件字符值到程序宏的转换
// Parameter: std::string val
//************************************
int CModbusCmdPara::TransDataTypeFromString(std::string val)
{
	int ret = -1;

	if (boost::iequals(strYxDataType,val))
	{
		ret = YxDataType;
	}
	else if (boost::iequals(strYcDataType,val))
	{
		ret = YcDataType;
	}
	else if (boost::iequals(strYkDataType,val))
	{
		ret = YkDataType;
	}
	else if (boost::iequals(strYmDataType,val))
	{
		ret = YmDataType;
	}
	else if (boost::iequals(strSynTimeType,val))
	{
		ret = SynTimeType;
	}

	return ret;
}

//************************************
// Method:    getbInit
// FullName:  Protocol::CModbusCmdPara::getbInit
// Access:    public 
// Returns:   bool
// Qualifier: 判断命令参数是否已经被初始化过
//************************************
//bool CModbusCmdPara::getbInit()
//{
//	return bInitialized_;
//}

//************************************
// Method:    getAddr
// FullName:  Protocol::CModbusCmdPara::getAddr
// Access:    public 
// Returns:   unsigned short
// Qualifier: 返回命令参数对应的通讯结点的地址
//************************************
typeAddr CModbusCmdPara::getAddr()
{
	return addr_;
}
};//namespace Protocol 
