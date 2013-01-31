#include "LinkNode.h"
#include "../DataBase/DAOperate.h"

namespace DistributedFA {

CLinkNode::CLinkNode(boost::asio::io_service & io_service,DataBase::CDAOperate & da_op)
					:COperateNode(da_op),
					CCommPoint(io_service),
					da_op_(da_op)
{
}

CLinkNode::~CLinkNode(void)
{
}

//bool CLinkNode::getCommAlive()
//{
//	if(da_op_.getYxVal(getCommIndex()))
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

//int CLinkNode::getCommIndex()
//{
//	return CommIndex_;
//}
//
//int CLinkNode::setCommIndex(int val)
//{
//	CommIndex_ = val;
//
//	return 0;
//}

int CLinkNode::getPosition()
{
	if (getCommActive())
	{
		return COperateNode::getPosition();
	}

	return -1;
}

int CLinkNode::getProtect()
{
	if (getCommActive())
	{
		return COperateNode::getProtect();
	}

	return -1;
}

int CLinkNode::OpenSwitchSel()
{
	if (getCommActive())
	{
		return COperateNode::OpenSwitchSel();
	}

	return -1;
}

int CLinkNode::OpenSwitchExe()
{
	if (getCommActive())
	{
		return COperateNode::OpenSwitchExe();
	}

	return -1;
}

int CLinkNode::CloseSwitchSel()
{
	if (getCommActive())
	{
		return COperateNode::CloseSwitchSel();
	}

	return -1;
}

int CLinkNode::CloseSwitchExe()
{
	if (getCommActive())
	{
		return COperateNode::CloseSwitchExe();
	}

	return -1;
}

share_commpoint_ptr CLinkNode::getSelfPtr()
{
	return share_commpoint_ptr(this);
}

SigConnection CLinkNode::ConnectSubAliveSig(CmdRecallSlotType slotVal)
{
	return da_op_.ConnectYkSig(slotVal);
}

SigConnection CLinkNode::ConnectSubTempSig(CmdRecallSlotType slotVal)
{
	return SigConnection();
}

int CLinkNode::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	int ret = DataBase::CCommPoint::LoadXmlCfg(xml);

	return ret;
}

void CLinkNode::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	DataBase::CCommPoint::SaveXmlCfg(xml);
}

}; //namespace DistributedFA

