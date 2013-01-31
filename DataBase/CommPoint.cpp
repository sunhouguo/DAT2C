#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include <boost/bind.hpp>
#include "CommPoint.h"
#include "../Protocol/CmdQueue.h"
#include "../CommInterface/CommFactory.h"
#include "../FileSystem/Markup.h"
#include "../CommInterface/CommInterface.h"
#include "../CommInterface/ServerInterface.h"

namespace DataBase {

const std::string comminterface_create_err = "不能创建通讯通道，通道参数可能非法";
const std::string comminterface_get_err = "不能获得通讯通道，通道参数可能非法";
const std::string comm_init_err = "不能初始化连接，规约参数可能非法";
//const std::string protocol_para_invalid = "未定义的规约参数";
//const std::string commchannel_para_invalid = "未定义的通道类型参数";
//const std::string ip_para_invalid = "未定义的IP参数";
//const std::string ip_para_missing = "未找到IP参数配置项";
//const std::string netPort_para_invalid = "未定义的网络端口参数";
//const std::string netPort_para_missing = "未找到网络端口参数配置项";
//const std::string serialPort_para_invalid = "未定义的串口端口参数";
//const std::string serialPort_para_missing = "未找到串口端口参数配置项";

const std::string strTerminalID = "ID";
const std::string strCommEventNO = "CommEventNO";
const std::string strYxEventNO = "YxEventNO";
const std::string strYkEventNO = "YkEventNO";
const std::string strSelfEventNO = "SelfEventNO";

CCommPoint::CCommPoint(boost::asio::io_service & io_service)
						  :io_service_(io_service),
						  timerRecv_(io_service)
{
	bCommActive_ = false;
	bFcbFlag_ = true;
	iLostAnswerTimes_ = 0;
	MulCmdRelaySigCounter_ = 0;

	bEncryptOutside_ = false;

	bInitCommPoint_ = false;
	bSynTCommPoint_ = false;
	bDelayDownload_ = false;

	CommRegisterNO_ = -1;
	YxReisterNO_ = -1;
	YkRegisterNO_ = -1;
	SelfRegisterNO_ = -1;

	td_diff_ = boost::posix_time::time_duration(0,0,0,0);
}

CCommPoint::~CCommPoint(void)
{
	//DisconnectCmdRelaySig();
}

typeAddr CCommPoint::getAddr()
{
	return addr_;
}

int CCommPoint::setAddr( typeAddr val )
{
	addr_ = val;

	return 0;
}

typeAddr CCommPoint::getBroadCastAddr()
{
	return BroadCastAddr_;
}

int CCommPoint::setBroadCastAddr( typeAddr val )
{
	BroadCastAddr_ = val;

	return 0;
}

bool CCommPoint::getbAcceptBroadCast()
{
	return bAceeptBroadcast_;
}

int CCommPoint::setbAcceptBroadCast(bool val)
{
	bAceeptBroadcast_ = val;

	return 0;
}

int CCommPoint::EnableCommunication(CmdRecallSlotType slotVal,share_pointval_ptr ptr /* = share_pointval_ptr() */,bool bNewPtr /* = true */)
{
	if (bNewPtr)
	{
		commPtr_.reset(CommInterface::CCommFactory::CreatCommInterface(commPara_,io_service_));
		//commPtr_ = CommInterface::CCommFactory::CreatCommInterface(commPara_,io_service_);
		if(commPtr_)
		{
			if(!commPtr_->InitConnect())
			{
				AddSelfPointToProtocol();
				ConnectCmdRecallSig(slotVal);
				commPtr_->OpenConnect();
			}
			else
			{
				commPtr_->UninitConnect();
				throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(comm_init_err);
			}
		}
		else
		{
			serverPtr_.reset(CommInterface::CCommFactory::CreateServerInterface(commPara_,io_service_));
			if (serverPtr_)
			{
				ConnectCmdRecallSig(slotVal);
				serverPtr_->InitServer(getSelfPtr());
				serverPtr_->StartServer();
			}
		}

		if ((!commPtr_) && (!serverPtr_))
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(comminterface_create_err);
		}
	}
	else
	{
		try
		{
			commPtr_ = boost::get<share_comm_ptr>(ptr);
			AddSelfPointToProtocol();
		}
		catch (boost::bad_get e)
		{
			ClearCommPtr();

			try
			{
				serverPtr_ = boost::get<share_server_ptr>(ptr);
				serverPtr_->InitServer(getSelfPtr());
			}
			catch (boost::bad_get e)
			{
				ClearServerPtr();
			}
		}

		if ((!commPtr_) && (!serverPtr_))
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(comminterface_get_err);
		}
	}

	return 0;
}

void CCommPoint::DisableCommunication()
{
	ClearCommPtr();
	ClearServerPtr();
}

void CCommPoint::ClearServerPtr()
{
	serverPtr_.reset();
}

void CCommPoint::SetServerPtr(share_server_ptr valPtr)
{
	serverPtr_ = valPtr;
}

share_server_ptr CCommPoint::getServerPtr()
{
	return serverPtr_;
}

share_comm_ptr CCommPoint::getCommPtr()
{
	return commPtr_;
}

void CCommPoint::ClearCommPtr()
{
	if (commPtr_)
	{
		if (commPtr_->getConnectAlive())
		{
			commPtr_->CloseConnect();
		}

		commPtr_->UninitConnect();
	}

	commPtr_.reset();
}

void CCommPoint::ResetCommPtr(CommInterface::CCommInterface * valPtr)
{
	if (commPtr_)
	{
		if (commPtr_->getConnectAlive())
		{
			commPtr_->CloseConnect();
		}

		commPtr_->UninitConnect();
	}

	commPtr_.reset(valPtr);
}

void CCommPoint::ResetCommPtr(share_comm_ptr valPtr)
{
	if (commPtr_)
	{
		if (commPtr_->getConnectAlive())
		{
			commPtr_->CloseConnect();
		}

		commPtr_->UninitConnect();
	}
	
	commPtr_.reset();

	commPtr_ = valPtr;
}

int CCommPoint::AddCmdVal(Protocol::CCmd val)
{
	if (commPtr_)
	{
		return commPtr_->AddCmdVal(val);
	}

	return -1;
	
}

int CCommPoint::AddCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint,boost::any val)
{
	Protocol::CCmd cmd(CmdType,priority,commpoint,val);

	return AddCmdVal(cmd);
}

int CCommPoint::AddCmdVal(typeCmd CmdType,unsigned char priority,share_commpoint_ptr commpoint)
{
	Protocol::CCmd cmd(CmdType,priority,commpoint);

	return AddCmdVal(cmd);
}

void CCommPoint::clearCmdQueue()
{
	if (commPtr_)
	{
		return commPtr_->clearCmdQueue();
	}
}

int CCommPoint::getCmdQueueSum()
{
	if (commPtr_)
	{
		return commPtr_->getCmdQueueSum();
	}

	return -1;
	
}

int CCommPoint::getMaxPriopriyCmd(Protocol::CCmd & dstVal)
{
	if (commPtr_)
	{
		return commPtr_->getMaxPriopriyCmd(dstVal);
	}

	return -1;
	
}

//comm api
bool CCommPoint::getCommActive()
{
	if(commPtr_)
	{
		return (commPtr_->getConnectAlive() && bCommActive_);
	}

	return false;
}

void CCommPoint::setCommActive(bool val)
{
	if (commPtr_)
	{
		bCommActive_ = commPtr_->getConnectAlive() & val;
	}
	else
	{
		bCommActive_ = false;
	}
	
	if (getCommActive())
	{
		iLostAnswerTimes_ = 0;
	}
}

bool CCommPoint::getInitCommPointFlag()
{
	return bInitCommPoint_;
}

void CCommPoint::setInitCommPointFlag(bool val)
{
	bInitCommPoint_ = val;
}

bool CCommPoint::getSynTCommPointFlag()
{
	return bSynTCommPoint_;
}

void CCommPoint::setSynTCommPointFlag(bool val)
{
	bSynTCommPoint_ = val;
}

bool CCommPoint::getDelayCommPointFlag()
{
	return bDelayDownload_;
}

void CCommPoint::setDelayCommPointFlag(bool val)
{
	bDelayDownload_ = val;
}

/*
bool CCommPoint::getDataActiveUp()
{
	if (commPtr_)
	{
		return commPtr_->getDataActiveUp();
	}

	return false;
}
*/

bool CCommPoint::CheckLostAnserTimesPlus( size_t LostAnswerTimesOut )
{
	if (iLostAnswerTimes_ >= LostAnswerTimesOut)
	{
		setCommActive(false);
		iLostAnswerTimes_ = 0;

		return true;
	}
	else
	{
		iLostAnswerTimes_++;

		return false;
	}
}

unsigned char CCommPoint::getCommPointType()
{
	return pointType_;
}

//cmd recall api
int CCommPoint::ConnectCmdRecallSig(CmdRecallSlotType slotVal)
{
	if (commPtr_ && getCommPointType() == TERMINAL_NODE)
	{
		return commPtr_->ConnectCmdRecallSig(slotVal);
	}
	else if (serverPtr_ && getCommPointType() == TERMINAL_NODE)
	{
		return serverPtr_->setCmdRecallSlot(slotVal);
	}

	return -1;
}

int CCommPoint::DisconnectCmdRecallSig()
{
	if (commPtr_)
	{
		return commPtr_->DisconnectCmdRecallSig();
	}

	return -1;
}

void CCommPoint::ClearMulCmdRelaySigCounter()
{
	MulCmdRelaySigCounter_ = 0;
}

void CCommPoint::MulCmdRelaySigCounterPlus()
{
	MulCmdRelaySigCounter_++;
}

bool CCommPoint::MulCmdRelaySigCounterCut()
{
	if (--MulCmdRelaySigCounter_ <= 0)
	{
		ClearMulCmdRelaySigCounter();

		return true;
	}

	return false;
}

int CCommPoint::ConnectCmdRelaySig(CmdRecallSlotType slotVal)
{
	if (!RelaySigConnection_.connected())
	{
		RelaySigConnection_ = RelaySig_.connect(slotVal);
	}
	
	if(!SubTempSigConnection_.connected())
	{
		SubTempSigConnection_ = ConnectSubTempSig(boost::bind(&CCommPoint::ProcessRelayCmd,getSelfPtr(),_1,_2,_3,_4));
	}

	MulCmdRelaySigCounterPlus();

	return 0;
}

int CCommPoint::DisconnectCmdRelaySig( bool bForceClose )
{
	if (bForceClose)
	{
		RelaySigConnection_.disconnect();
		SubTempSigConnection_.disconnect();

		ClearMulCmdRelaySigCounter();
	}
	else
	{
		if(MulCmdRelaySigCounterCut())
		{
			RelaySigConnection_.disconnect();
			SubTempSigConnection_.disconnect();
		}
	}

	return 0;
}

//bool CCommPoint:: getSubTempSigConnection()
//{
//	return SubTempSigConnection_.connected();
//}

void CCommPoint::ProcessRelayCmd(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	RelaySig_(cmdType,ReturnCode,getSelfPtr(),val);
}

void CCommPoint::AddSelfPointToProtocol()
{
	if (commPtr_)
	{
		commPtr_->AddCommPoint(getSelfPtr());
	}

	//ConnectCmdRecallSig();
}

//void CCommPoint::AddSelfPointToServerPtr()
//{
//	if (serverPtr_)
//	{
//		serverPtr_->AddCommPoint(getSelfPtr());
//	}
//}

int CCommPoint::ResetTimerRecv(size_t LostRecvTimeOut)
{
	timerRecv_.expires_from_now(boost::posix_time::seconds(LostRecvTimeOut));
	timerRecv_.async_wait(boost::bind(&CCommPoint::handle_timerRecv,getSelfPtr(),boost::asio::placeholders::error));

	return 0;
}

void CCommPoint::handle_timerRecv(const boost::system::error_code& error)
{
	if (!error)
	{
		setCommActive(false);
	}
}

void CCommPoint::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddElem(strTerminalID,getRemoteID());

	if (CommRegisterNO_ > 0)
	{
		xml.AddElem(strCommEventNO,CommRegisterNO_);
	}

	if (YxReisterNO_ > 0)
	{
		xml.AddElem(strYxEventNO,YxReisterNO_);
	}

	if (YkRegisterNO_ > 0)
	{
		xml.AddElem(strYkEventNO,YkRegisterNO_);
	}

	if (SelfRegisterNO_ > 0)
	{
		xml.AddElem(strSelfEventNO,SelfRegisterNO_);
	}

	commPara_.SaveXmlCfg(xml);
}

int CCommPoint::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	xml.ResetMainPos();
	if (xml.FindElem(strTerminalID))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);

		setRemoteID(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strCommEventNO))
	{
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			CommRegisterNO_ = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的通讯事件注册号参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYxEventNO))
	{
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			YxReisterNO_ = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的遥信事件注册号参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYkEventNO))
	{
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			YkRegisterNO_ = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的遥控事件注册号参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strSelfEventNO))
	{
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			SelfRegisterNO_ = boost::lexical_cast<int>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的遥测事件注册号参数:"<<e.what();

			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
	}

	xml.ResetMainPos();
	int ret = commPara_.LoadXmlCfg(xml);

	return ret;
}

std::string CCommPoint::getLocalPort()
{
	return commPara_.getLocalPort();
}

std::string CCommPoint::getRemoteID()
{
	return commPara_.getRemoteID();
}

int CCommPoint::setRemoteID(std::string val)
{
	commPara_.setRemoteID(val);

	return 0;
}

bool CCommPoint::MatchRemoteID(std::string val)
{
	boost::trim(val);

	return boost::iequals(commPara_.getRemoteID(),val);
}

bool CCommPoint::MatchRemoteIP(std::string val)
{
	return commPara_.MatchRemoteIP(val);
}

//void CCommPoint::ClearDataBaseQuality(bool active)
//{
//	return;//do nothing
//}

CommInterface::CCommPara & CCommPoint::getCommPara()
{
	return commPara_;
}

int CCommPoint::ConnectEventStatusSig(CmdRecallSlotType slotVal)
{
	if (!EventStatusConnection_.connected())
	{
		EventStatusConnection_ = EventStatusSig_.connect(slotVal);
	}

	return 0;
}

void CCommPoint::DisconnectEventStatusSig()
{
	EventStatusConnection_.disconnect();
}

int CCommPoint::NotifyEventStatus(typeCmd EventType,unsigned char ReturnCode)
{
	if(EventStatusConnection_.connected())
	{
		int EventNO = -1;

		switch(EventType)
		{
		case COMM_EVENT:
			EventNO = CommRegisterNO_;
			break;

		case YX_EVENT:
			EventNO = YxReisterNO_;
			break;

		case YK_EVENT:
			EventNO = YkRegisterNO_;
			break;

		case SELF_EVENT:
			EventNO = SelfRegisterNO_;
			break;

		default:
			break;
		}

		if(EventNO > 0)
		{
			EventStatusSig_(EventType,ReturnCode,getSelfPtr(),EventNO);

			return 0;
		}
	}

	return -1;
}

int CCommPoint::InitLocalServices( CmdRecallSlotType slotVal,bool SlotReady )
{
	if(SlotReady)
	{
		if (CommRegisterNO_ > 0 || YxReisterNO_ > 0 || YkRegisterNO_ > 0 || SelfRegisterNO_ > 0)
		{
			ConnectEventStatusSig(slotVal);
		}
	}

	return 0;
}

void CCommPoint::UnInitLocalServices()
{
	DisconnectEventStatusSig();
}

boost::posix_time::time_duration CCommPoint::getTdDiff()
{
	return td_diff_;
}

int CCommPoint::setTdDiff(boost::posix_time::time_duration val)
{
	td_diff_ = val;

	return 0;
}

//encrypt api
bool CCommPoint::getEncryptOutside()
{
	return bEncryptOutside_;
}

int CCommPoint::setEncryptOutside(bool val)
{
	bEncryptOutside_ = val;

	return 0;
}

} //namespace DataBase


