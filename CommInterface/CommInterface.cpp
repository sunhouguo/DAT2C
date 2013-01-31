#include "CommInterface.h"
#include <boost/exception/all.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "../Protocol/Protocol.h"
#include "../Protocol/ProtocolFactory.h"
#include "../FileSystem/LogFactory.h"
#include "../FileSystem/Log.h"

namespace CommInterface {

const std::string no_ptr_exsit = "No such ptr exsit";

CCommInterface::CCommInterface(CCommPara & para,boost::asio::io_service & io_service)
							  :para_(para),
							  io_service_(io_service)
{
	
}

CCommInterface::~CCommInterface(void)
{
}

//comm points api
int CCommInterface::AddCommPoint(share_commpoint_ptr val)
{
	if (protocol_)
	{
		return protocol_->AddCommPoint(val);
	}

	return -1;

}

int CCommInterface::DelCommPoint(int index)
{
	if (protocol_)
	{
		return protocol_->DelCommPoint(index);
	}

	return -1;
}

weak_commpoint_ptr CCommInterface::getCommPoint(int index)
{
	if (protocol_)
	{
		return protocol_->getCommPoint(index);
	}

	throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(no_ptr_exsit);

}

int CCommInterface::getCommPointSum()
{
	if (protocol_)
	{
		return protocol_->getCommPointSum();
	}

	return -1;

}

void CCommInterface::ClearCommPoint()
{
	if (protocol_)
	{
		return protocol_->ClearCommPoint();
	}

}

//protocol api
int CCommInterface::EnableProtocol()
{
	protocol_.reset(Protocol::CProtocolFactory::CreateProtocol(para_.getProtocolType(),io_service_));

	if (protocol_)
	{
		ConnectProtocolSingal();

		if (para_.getEnableSpecialProtocolCfg())
		{
			return protocol_->LoadXmlCfg(para_.getProtocolCfgFileName());
		}
	}
	else
	{
		return -1;
	}
	
	return 0;
}

void CCommInterface::DisableProtocol()
{
	DisconnectProtocolSingal();

	protocol_.reset();
}

int CCommInterface::InitProtocol()
{
	if (protocol_)
	{
		return protocol_->InitProtocol();
	}

	return	-1;
}

void CCommInterface::UninitProtocol( bool bDisableCommpoint /*= true*/ )
{
	if (protocol_)
	{
		protocol_->UninitProtocol();

		if(bDisableCommpoint)
		{
			protocol_->DisableAllCommPoints();
		}
	}
}

int CCommInterface::ProcessProtocolSingal(unsigned char commType,unsigned char * buf,size_t length)
{
	int ret = -1;

	switch (commType)
	{
	case Protocol::WriteData:
		ret = WriteToConnect(buf,length);
		break;

	case Protocol::ReadData:
		ret = ReadFromConnect(buf,length);
		break;

	case Protocol::OpenConnect:
		ret = OpenConnect();
		break;

	case Protocol::CloseConnect:
		ret = CloseConnect();
		break;

	case Protocol::ReConnect:
		ret = RetryConnect();
		break;

	case Protocol::BroadCast:
		ret = WriteToBroadCast(buf,length);
		break;

	default:
		break;
	}

	return ret;
}

int CCommInterface::ConnectProtocolSingal()
{
	DisconnectProtocolSingal();

	if (protocol_)
	{
		ProtocolSingalConnection_ = protocol_->ConnectCommSingal(boost::bind(&CCommInterface::ProcessProtocolSingal,this,_1,_2,_3));
	}

	return 0;
}
int CCommInterface::DisconnectProtocolSingal()
{
	if (ProtocolSingalConnection_.connected())
	{
		ProtocolSingalConnection_.disconnect();
	}

	return 0;
}

/*
int CCommInterface::setProtocoType(unsigned short val)
{
	protocolType_ = val;

	return 0;
}
*/

int CCommInterface::AddCmdVal(Protocol::CCmd val)
{
	if (protocol_)
	{
		return protocol_->AddSendCmdVal(val);
	}

	return -1;

}

void CCommInterface::clearCmdQueue()
{
	if (protocol_)
	{
		return protocol_->ClearSendCmdQueue();
	}

}

int CCommInterface::getCmdQueueSum()
{
	if (protocol_)
	{
		return protocol_->getSendCmdQueueSum();
	}

	return -1;

}

int CCommInterface::getMaxPriopriyCmd(Protocol::CCmd & dstVal)
{
	if (protocol_)
	{
		return protocol_->getMaxPriopriySendCmd(dstVal);
	}

	return -1;

}

/*
bool CCommInterface::getDataActiveUp()
{
	if (protocol_)
	{
		return protocol_->getDataActiveUp();
	}

	return false;
}
*/

//log ptr enable
int CCommInterface::EnableStatusLog( std::string filename,std::string filetype /*= strXmlLog*/,std::string limit /*=""*/ )
{
	statusLog_.reset(FileSystem::CLogFactory::CreateLog(filename,filetype,limit));

	return 0;
}

int CCommInterface::EnableFrameLog( std::string filename,std::string filetype /*= strXmlLog*/,std::string limit /*=""*/ )
{
	frameLog_.reset(FileSystem::CLogFactory::CreateLog(filename,filetype,limit));

	return 0;
}

void CCommInterface::DisableStatusLog()
{
	statusLog_.reset();
}

void CCommInterface::DisableFrameLog()
{
	frameLog_.reset();
}

//log api
int CCommInterface::AddStatusLog(std::string strVal)
{
	if (para_.getPrintStatusTerm())
	{
		PrintInfoToTerm(strVal);
	}

	if (statusLog_)
	{
		return statusLog_->AddRecord(strVal);
	}

	return -1;
}

int CCommInterface::AddStatusLogWithSynT(std::string strVal)
{
	if (para_.getPrintStatusTerm())
	{
		PrintInfoToTerm(strVal);
	}

	if (statusLog_)
	{
		return statusLog_->AddRecordWithSynT(strVal);
	}

	return -1;
}

int CCommInterface::AddFrameLog(std::string strVal)
{
	if (frameLog_)
	{
		return frameLog_->AddRecord(strVal);
	}

	return -1;
}

int CCommInterface::AddFrameLogWithSynT(std::string strVal)
{
	if (frameLog_)
	{
		return frameLog_->AddRecordWithSynT(strVal);
	}

	return -1;
}

int CCommInterface::PrintInfoToTerm(std::string strVal)
{
	if (para_.getPrintFrameTerm()|| para_.getPrintStatusTerm())
	{
		std::cout<<strVal;
		
		return 0;
	}

	return -1;
}

int CCommInterface::RecordFrameData( const unsigned char * buf,size_t count,bool bRecv )
{
	if (frameLog_ || para_.getPrintFrameTerm())
	{
		if (count > 0)
		{
			std::ostringstream ostr;

			if (bRecv)
			{
				ostr<<"Recv: ";
			}
			else
			{
				ostr<<"Send: ";
			}

			for (size_t i=0;i<count;i++)
			{
				ostr<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<(short)buf[i];
			}

			ostr<<std::endl;

			if (frameLog_)
			{
				AddFrameLogWithSynT(ostr.str());
			}
			
			if (para_.getPrintFrameTerm())
			{
				PrintInfoToTerm(ostr.str());
			}

			return 0;
		}
	}

	return -1;
}

//int CCommInterface::LoadProtocolXmlCfg(std::string filename)
//{
//	if (protocol_)
//	{
//		return protocol_->LoadXmlCfg(filename);
//	}
//
//	return -1;
//}
//
//void CCommInterface::SaveProtocolXmlCfg(std::string filename)
//{
//	if (protocol_)
//	{
//		protocol_->SaveXmlCfg(filename);
//	}
//}

//cmd recall api
int CCommInterface::ConnectCmdRecallSig(CmdRecallSlotType slotVal)
{
	if (protocol_)
	{
		return protocol_->ConnectCmdRecallSig(slotVal);
	}

	return -1;
}

int CCommInterface::DisconnectCmdRecallSig()
{
	if (protocol_)
	{
		return protocol_->DisconnectCmdRecallSig();
	}

	return -1;
}

int CCommInterface::WriteToBroadCast(const unsigned char * buf,size_t bytes_transferred)
{
	return 0;
}

int CCommInterface::RetryConnect()
{
	if (getConnectAlive())
	{
		CloseConnect();
	}

	OpenConnect();

	return 0;
}

bool CCommInterface::getConnectAlive()
{
	return true;
}

int CCommInterface::EnableLog()
{
	if (para_.getEnableChannelStatusLog())
	{
		EnableStatusLog(para_.getCommChannelStatusLogFileName(),para_.getCommChannelStatusLogType(),para_.getCommChannelStatusLogLimit());
	}

	if (para_.getEnableChannelFrameLog())
	{
		EnableFrameLog(para_.getCommChannelFrameLogFileName(),para_.getCommChannelFrameLogType(),para_.getCommChannelFrameLogLimit());
	}

	return 0;
}

void CCommInterface::DisableLog()
{
	DisableStatusLog();

	DisableFrameLog();
}

} //namespace CommInterface

