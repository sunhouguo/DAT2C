#include "SelfNode.h"
#include "LinkNode.h"
#include "LoadSwtich.h"

namespace DistributedFA {

CSelfNode::CSelfNode(DataBase::CDAOperate & da_op)
{
	ChangeNodeType(new CLoadSwtich(da_op));
}

CSelfNode::~CSelfNode(void)
{
}

bool CSelfNode::CheckTieSwitch()
{
	if (powerNode_ && loadNode_) //电源侧和负荷侧都有邻接点
	{
		if ((powerNode_->getPosition() == PositionClose) && (getPosition() == PositionOpen) && (loadNode_->getPosition() == PositionClose)) //靠两边开关位置和自己的开关位置3者判断自己是否为联络开关
		{
			return true;
		}
	}
	else if (powerNode_ && !loadNode_) //电源侧有邻接点而负荷侧没有，即自身是线路尾端结点
	{
		if ((powerNode_->getPosition() == PositionClose) && (getPosition() == PositionOpen)) //靠单边开关位置和自己的开关位置2者判断自己是否为联络开关
		{
			return true;
		}
	}
	else if (loadNode_ && !powerNode_) //负荷侧有邻接点而电源侧没有，即自身是线路头端结点
	{
		if ((getPosition() == PositionOpen) && (loadNode_->getPosition() == PositionClose)) //靠单边开关位置和自己的开关位置2者判断自己是否为联络开关
		{
			return true;
		}
	}

	return false;
}

bool CSelfNode::CheckSelfFault()
{
	if (powerNode_ && loadNode_) //电源侧和负荷侧都有邻接点
	{
		if ((powerNode_->getPosition() == PositionClose) && (getPosition() == PositionClose) && (loadNode_->getPosition() == PositionClose)) //开关位置 1 1 1
		{
			if ((powerNode_->getProtect() == ProtectedPositive) && (getProtect() == ProtectedPositive) && (loadNode_->getProtect() == ProtectedNagetive))     //保护信号 1 1 0
			{
				return true;
			}
			else if((powerNode_->getProtect() == ProtectedNagetive) && (getProtect() == ProtectedPositive) && (loadNode_->getProtect() == ProtectedPositive)) //保护信号 0 1 1
			{
				return true;
			}
			else if((powerNode_->getProtect() == ProtectedPositive) && (getProtect() == ProtectedNagetive) && (loadNode_->getProtect() == ProtectedNagetive)) //保护信号 1 0 0
			{
				return true;
			}
			else if((powerNode_->getProtect() == ProtectedNagetive) && (getProtect() == ProtectedNagetive) && (loadNode_->getProtect() == ProtectedPositive)) //保护信号 0 0 1
			{
				return true;
			}
		}
		else if ((powerNode_->getPosition() == PositionOpen) && (getPosition() == PositionClose) && (loadNode_->getPosition() == PositionClose)) //开关位置 0 1 1
		{
			if((powerNode_->getProtect() == ProtectedNagetive) && (getProtect() == ProtectedPositive) && (loadNode_->getProtect() == ProtectedPositive))      //保护信号 0 1 1
			{
				return true;
			}
			else if((powerNode_->getProtect() == ProtectedNagetive) && (getProtect() == ProtectedNagetive) && (loadNode_->getProtect() == ProtectedPositive)) //保护信号 0 0 1
			{
				return true;
			}
		}
		else if((powerNode_->getPosition() == PositionClose) && (getPosition() == PositionClose) && (loadNode_->getPosition() == PositionOpen)) //开关位置 1 1 0
		{
			if ((powerNode_->getProtect() == ProtectedPositive) && (getProtect() == ProtectedPositive) && (loadNode_->getProtect() == ProtectedNagetive))     //保护信号 1 1 0
			{
				return true;
			}
			else if((powerNode_->getProtect() == ProtectedPositive) && (getProtect() == ProtectedNagetive) && (loadNode_->getProtect() == ProtectedNagetive)) //保护信号 1 0 0
			{
				return true;
			}
		}
	}
	else if (powerNode_ && !loadNode_) //电源侧有邻接点而负荷侧没有，即自身是线路尾端结点
	{
		if ((powerNode_->getPosition() == PositionClose) && (getPosition() == PositionClose)) //开关位置 1 1
		{
			if ((powerNode_->getProtect() == ProtectedPositive) && (getProtect() == ProtectedPositive))      //保护信号 1 1
			{
				return true;
			}
			else if ((powerNode_->getProtect() == ProtectedPositive) && (getProtect() == ProtectedNagetive)) //保护信号 1 0
			{
				return true;
			}
		}
	}
	else if (loadNode_ && !powerNode_) //负荷侧有邻接点而电源侧没有，即自身是线路头端结点
	{
		if ((getPosition() == PositionClose) && loadNode_->getPosition() == PositionClose) //开关位置 1 1
		{
			if (getProtect() == ProtectedPositive && loadNode_->getProtect() == ProtectedNagetive)          //保护信号 1 0
			{
				return true;
			}
		}
		else if ((getPosition() == PositionClose) && loadNode_->getPosition() == PositionOpen) //开关位置 1 0
		{
			if (getProtect() == ProtectedPositive && loadNode_->getProtect() == ProtectedNagetive)          //保护信号 1 0
			{
				return true;
			}
		}
	}

	return false;
}

int CSelfNode::getPosition()
{
	if (selfNodeType_)
	{
		return selfNodeType_->getPosition();
	}

	return -1;
}

int CSelfNode::getProtect()
{
	if (selfNodeType_)
	{
		return selfNodeType_->getProtect();
	}

	return -1;
}

int CSelfNode::OpenSwitchSel()
{
	if (selfNodeType_)
	{
		return selfNodeType_->OpenSwitchSel();
	}

	return -1;
}

int CSelfNode::OpenSwitchExe()
{
	if (selfNodeType_)
	{
		return selfNodeType_->OpenSwitchExe();
	}

	return -1;
}

int CSelfNode::CloseSwitchSel()
{
	if (selfNodeType_)
	{
		return selfNodeType_->CloseSwitchSel();
	}
	
	return -1;
}

int CSelfNode::CloseSwitchExe()
{
	if (selfNodeType_)
	{
		return selfNodeType_->CloseSwitchExe();
	}

	return -1;
}

void CSelfNode::ChangeNodeType(COperateNode * nodeType)
{
	selfNodeType_.reset(nodeType);
}

}; //namespace DistributedFA
