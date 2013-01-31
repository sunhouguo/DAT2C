#include "CmdQueue.h"
#include <boost/bind.hpp>

namespace Protocol {

CCmd::CCmd()
{

}

CCmd::CCmd(typeCmd CmdType,
		   unsigned char priority,
		   share_commpoint_ptr commpoint)
		   :CmdType_(CmdType),
		   priority_(priority),
		   commpoint_(commpoint)

{

}

CCmd::CCmd(typeCmd CmdType,
		   unsigned char priority,
		   share_commpoint_ptr commpoint,
		   boost::any val)
		   :CmdType_(CmdType),
		   priority_(priority),
		   commpoint_(commpoint),
		   val_(val)

{

}

CCmd::~CCmd(void)
{

}

unsigned char CCmd::getPriority()
{
	return priority_;
}

typeCmd CCmd::getCmdType()
{
	return CmdType_;
}

boost::any CCmd::getVal()
{
	return val_;
}

share_commpoint_ptr CCmd::getCommPoint()
{
	return commpoint_;
}

int CCmd::setCommPoint(share_commpoint_ptr val)
{
	commpoint_.reset();

	commpoint_ = val;

	return 0;
}

CCmdQueue::CCmdQueue(void)
{
}

CCmdQueue::~CCmdQueue(void)
{
}

int CCmdQueue::getSendCmdQueueSum()
{
	//SendCmdMutex_.lock();

	return SendCmdQueue_.size();
	
	//SendCmdMutex_.unlock();

	//return ret;
}

void CCmdQueue::ClearSendCmdQueue()
{
	boost::unique_lock<boost::mutex> uLock(SendCmdMutex_);

	SendCmdQueue_.clear();
}

int CCmdQueue::getWaitCmdQueueSum()
{
	//WaitCmdMutex_.lock();

	return WaitCmdQueue_.size();
	
	//WaitCmdMutex_.unlock();

	//return ret;
}

int CCmdQueue::getWaitCmdQueueCount(share_commpoint_ptr commpoint)
{
	int count = 0;

	if (commpoint)
	{
		for (int i=0;i<getWaitCmdQueueSum();i++)
		{
			share_commpoint_ptr TempPoint = WaitCmdQueue_[i].getCommPoint();
			if (TempPoint)
			{
				if (TempPoint == commpoint)
				{
					count++;
				}
			}
		}
	}
	
	return count;
}

void CCmdQueue::ClearWaitCmdQueue()
{
	boost::unique_lock<boost::mutex> uLock(WaitCmdMutex_);

	WaitCmdQueue_.clear();
}

int CCmdQueue::AddSendCmdVal(CCmd val)
{
	boost::unique_lock<boost::mutex> uLock(SendCmdMutex_);

	SendCmdQueue_.push_back(val);

	return 0;
}

int CCmdQueue::AddSendCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint)
{
	CCmd cmdVal(CmdType,priority,commpoint);

	return AddSendCmdVal(cmdVal);
}

int CCmdQueue::AddSendCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val)
{
	CCmd cmdVal(CmdType,priority,commpoint,val);

	return AddSendCmdVal(cmdVal);
}

int CCmdQueue::AddWaitCmdVal(CCmd val)
{
	boost::unique_lock<boost::mutex> uLock(WaitCmdMutex_);

	WaitCmdQueue_.push_back(val);
	
	return 0;
}

int CCmdQueue::AddWaitCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint)
{
	CCmd cmdVal(CmdType,priority,commpoint);

	return AddWaitCmdVal(cmdVal);
}

int CCmdQueue::AddWaitCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val)
{
	CCmd cmdVal(CmdType,priority,commpoint,val);

	return AddWaitCmdVal(cmdVal);
}

int CCmdQueue::AddOnlyWaitCmdWithoutVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val)
{
	for(std::vector<CCmd>::iterator it = WaitCmdQueue_.begin();it != WaitCmdQueue_.end();it++)
	{
		if (CompareCmdEqual(CmdType,priority,commpoint,(*it).getCmdType(),(*it).getPriority(),(*it).getCommPoint()))
		{
			return 1;
		}
	}

	return AddWaitCmdVal(CmdType,priority,commpoint,val);
}

int CCmdQueue::AddOnlyWaitCmdByCmdType(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val)
{
	for(std::vector<CCmd>::iterator it = WaitCmdQueue_.begin();it != WaitCmdQueue_.end();it++)
	{
		if (CompareCmdEqual(CmdType,(*it).getCmdType()))
		{
			return 1;
		}
	}

	return AddWaitCmdVal(CmdType,priority,commpoint,val);
}

bool CCmdQueue::CompareCmdEqual( typeCmd srcCmdType,unsigned char srcPriority,share_commpoint_ptr srcCommpoint, typeCmd dstCmdType,unsigned char dstPriority,share_commpoint_ptr dstCommpoint )
{
	if((srcCmdType == dstCmdType)
		&& (srcPriority == dstPriority)
		&& (srcCommpoint && dstCommpoint && (srcCommpoint == dstCommpoint)))
	{
		return true;
	}

	return false;
}

bool CCmdQueue::CompareCmdEqual(typeCmd srcCmdType,typeCmd dstCmdType)
{
	if (srcCmdType == dstCmdType)
	{
		return true;
	}

	return false;
}

bool CCmdQueue::SearchSendCmdQueue(typeCmd CmdType,unsigned char Priority,share_commpoint_ptr Commpoint)
{
	for(std::vector<CCmd>::iterator it = SendCmdQueue_.begin();it != SendCmdQueue_.end();it++)
	{
		if (CompareCmdEqual(CmdType,Priority,Commpoint,(*it).getCmdType(),(*it).getPriority(),(*it).getCommPoint()))
		{
			return true;
		}
	}

	return false;
}

bool CCmdQueue::SearchSendCmdQueue(typeCmd CmdType)
{
	for(std::vector<CCmd>::iterator it = SendCmdQueue_.begin();it != SendCmdQueue_.end();it++)
	{
		if (CompareCmdEqual(CmdType,(*it).getCmdType()))
		{
			return true;
		}
	}

	return false;
}

//bool CCmdQueue::ComparePrioprity(CCmd cmd1,CCmd cmd2)
//{
//	return cmd1.getPriority() > cmd2.getPriority();
//}

int CCmdQueue::getMaxPriopriySendCmd(CCmd & dstVal)
{
	if (getSendCmdQueueSum() <= 0)
	{
		return -1;
	}

	unsigned char HighPriority = 0;
	size_t resultIndex  = 0;

	for (int i=0;i<getSendCmdQueueSum();i++)
	{
		if (SendCmdQueue_[i].getPriority() > HighPriority)
		{
			HighPriority = SendCmdQueue_[i].getPriority();
			resultIndex = i;
		}
	}

	dstVal = SendCmdQueue_[resultIndex];

	boost::unique_lock<boost::mutex> uLock(SendCmdMutex_);

	SendCmdQueue_.erase(SendCmdQueue_.begin() + resultIndex);

	return 0;
}

int CCmdQueue::getMaxPriopriyWaitCmd(CCmd & dstVal)
{
	if (getWaitCmdQueueSum() <= 0)
	{
		return -1;
	}

	unsigned char HighPriority = 0;
	size_t resultIndex  = 0;

	for (int i=0;i<getWaitCmdQueueSum();i++)
	{
		if (WaitCmdQueue_[i].getPriority() > HighPriority)
		{
			HighPriority = WaitCmdQueue_[i].getPriority();
			resultIndex = i;
		}
	}

	dstVal = WaitCmdQueue_[resultIndex];

	boost::unique_lock<boost::mutex> uLock(WaitCmdMutex_);

	WaitCmdQueue_.erase(WaitCmdQueue_.begin() + resultIndex);
	
	return 0;
}

int CCmdQueue::getMaxPriopriyWaitCmdByPointPtr(share_commpoint_ptr ptr,CCmd & dstVal)
{
	if (getWaitCmdQueueSum() <= 0)
	{
		return -1;
	}

	unsigned char HighPriority = 0;
	int resultIndex  = -1;

	for (int i=0;i<getWaitCmdQueueSum();i++)
	{
		if (WaitCmdQueue_[i].getCommPoint() == ptr)
		{
			if (WaitCmdQueue_[i].getPriority() > HighPriority)
			{
				HighPriority = WaitCmdQueue_[i].getPriority();
				resultIndex = i;
			}
		}
	}

	if (resultIndex >= 0)
	{
		dstVal = WaitCmdQueue_[resultIndex];

		boost::unique_lock<boost::mutex> uLock(WaitCmdMutex_);

		WaitCmdQueue_.erase(WaitCmdQueue_.begin() + resultIndex);
		
		return 0;
	}
	

	return -1;
}

}; //namespace Protocol {
