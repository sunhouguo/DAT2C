#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/exception/all.hpp>
#include "ServerInterface.h"
#include "CommPara.h"

namespace CommInterface {

CServerInterface::CServerInterface(CCommPara & para,boost::asio::io_service& io_service)
								   :para_(para),
								   io_service_(io_service)
{
}

CServerInterface::~CServerInterface(void)
{
}

int CServerInterface::AddCommPoint(share_commpoint_ptr val)
{
	commPoints_.push_back(weak_commpoint_ptr(val));

	return 0;
}

int CServerInterface::DelCommPoint(int index)
{
	if (index < 0 || index >= getCommPointSum())
	{
		return -1;
	}

	commPoints_.erase(commPoints_.begin() + index);

	return 0;
}

weak_commpoint_ptr CServerInterface::getCommPoint(int index)
{
	if (index < 0 || index >= getCommPointSum())
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	return commPoints_.at(index);
}

int CServerInterface::getCommPointSum()
{
	return commPoints_.size();
}

void CServerInterface::ClearCommPoint()
{
	commPoints_.clear();
}

int CServerInterface::setCmdRecallSlot(CmdRecallSlotType slotVal)
{
	CmdRecallSlot_.reset(new CmdRecallSlotType(slotVal));

	return 0;
}

int CServerInterface::TransMatchTypeFromString(std::string val)
{
	int ret = -1;

	if (boost::iequals(strMatchByID,val))
	{
		ret = ID_MATCHING;
	}
	else if (boost::iequals(strMatchByIP,val))
	{
		ret = IP_MATCHING;
	}

	return ret;
}

std::string CServerInterface::TransMatchTypeToString(int val)
{
	std::string strRet;

	switch (val)
	{
	case ID_MATCHING:
		strRet = strMatchByID;
		break;

	case IP_MATCHING:
		strRet = strMatchByIP;
		break;

	default:
		strRet = "";
		break;
	}

	return strRet;
}

int CServerInterface::InitServer(share_commpoint_ptr comm_point /*= share_commpoint_ptr()*/)
{
	AddCommPoint(comm_point);

	return 0;
}

};//namespace CommInterface 
