#include "OperateNode.h"
#include "../DataBase/DAOperate.h"

namespace DistributedFA {

COperateNode::COperateNode(DataBase::CDAOperate & da_op)
:da_op_(da_op)
{
}

COperateNode::~COperateNode(void)
{
}

int COperateNode::getPositionIndex()
{
	return PositionYxIndex_;
}

int COperateNode::getProtectIndex()
{
	return ProtectYxIndex_;
}

int COperateNode::setPositionIndex(int val)
{
	PositionYxIndex_ = val;

	return 0;
}

int COperateNode::setProtectIndex(int val)
{
	ProtectYxIndex_ = val;

	return 0;
}

int COperateNode::getPosition()
{
	if(da_op_.getYxVal(getPositionIndex()))
	{
		return PositionClose;
	}
	else
	{
		return PositionOpen;
	}
}

int COperateNode::getProtect()
{
	if(da_op_.getYxVal(getProtectIndex()))
	{
		return ProtectedPositive;
	}
	else
	{
		return ProtectedNagetive;
	}
}

int COperateNode::OpenSwitchSel()
{
	return da_op_.AddYkSelCmd(PositionYkIndex_,false);
}

int COperateNode::OpenSwitchExe()
{
	return da_op_.AddYkExeCmd(PositionYkIndex_,false);
}

int COperateNode::CloseSwitchSel()
{
	return da_op_.AddYkSelCmd(PositionYkIndex_,true);
}

int COperateNode::CloseSwitchExe()
{
	return da_op_.AddYkExeCmd(PositionYkIndex_,true);
}

}; //namespace DistributedFA
