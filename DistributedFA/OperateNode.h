#pragma once
#include "SWNode.h"

namespace DataBase
{
	class CDAOperate;
}

namespace DistributedFA {

/**
* 开关结点类，使用开关结点接口的声明
*/
class COperateNode
	:public CSWNode
{
public:
	COperateNode(DataBase::CDAOperate & da_op);
	virtual ~COperateNode(void);

	virtual int getPosition();
	virtual int getProtect();
	virtual int OpenSwitchSel();
	virtual int OpenSwitchExe();
	virtual int CloseSwitchSel();
	virtual int CloseSwitchExe();

private:
	int getPositionIndex();
	int getProtectIndex();
	int setPositionIndex(int val);
	int setProtectIndex(int val);

private:
	//self attrib
	int PositionYxIndex_;
	int ProtectYxIndex_;
	int PositionYkIndex_;
	DataBase::CDAOperate & da_op_;
};

}; //namespace DistributedFA
