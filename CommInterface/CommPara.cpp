#include <boost/exception/all.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "CommPara.h"
#include "../PublicSupport/Dat2cTypeDef.h"
#include "../FileSystem/Markup.h"

namespace CommInterface {

#define	strMatchType "MatchType"
#define strCommChannelType "CommChannel"
#define strChannelStatusLog "CommChannelStatusLog"
#define strChannelFrameLog "CommChannelFrameLog"
#define strLocalIP "LocalIP"
#define strLocalPort "LocalPort"
#define strRemoteIP "RemoteIP"
#define strRemotePort "RemotePort"
#define strMulticastIP "MulticastIP"
#define strMulticastPort "MulticastPort"
#define strBroadcastIP "BroadcastIP"
#define strBroadcastPort "BroadcastPort"
#define strProtocolType "Protocol"
#define strProtocolSpecialCfg "ProtoclCfgFile"
#define strPrintFrameTerm "PrintFrameTerm"
#define strPrintStatusTerm "PrintStatusTerm"
#define strDisableCommpointByCloseConnec "DisableCommpoints"

const std::string commchannel_para_missing = "未找到通道类型参数配置项";
const std::string protocol_para_missing = "未找到规约参数配置项";

CCommPara::CCommPara(void)
{
	bPrintFrame_ = false;
	bPrintStatus_ = false;
	commChannelFrameLog_ = "";
	commChannelFrameLogLimit_ = "";
	commChannelFrameLogType_ = "";
	commChannelStatusLog_ = "";
	commChannelStatusLogLimit_ = "";
	commChannelStatusLogType_ = "";
	commChannelType_ = "";
	commServerMatchType_ = "";
	localIP_ = "";
	localPort_ = "";
	protocolCfgFile_ = "";
	protocolType_ = "";
	remoteID_ = "";
	remoteParaIndex_ = 0;
	remoteIPs_.clear();
	remotePorts_.clear();

	DisableCommpointByCloseConnect_ = false;
}

CCommPara::CCommPara(const CCommPara & rhs)
{
	bPrintFrame_ = rhs.bPrintFrame_;
	bPrintStatus_ = rhs.bPrintStatus_;
	broadcastIP_ = rhs.broadcastIP_;
	broadcastPort_ = rhs.broadcastPort_;
	commChannelFrameLog_ = rhs.commChannelFrameLog_;
	commChannelFrameLogLimit_ = rhs.commChannelFrameLogLimit_;
	commChannelFrameLogType_ = rhs.commChannelFrameLogType_;
	commChannelStatusLog_ = rhs.commChannelStatusLog_;
	commChannelStatusLogLimit_ = rhs.commChannelStatusLogLimit_;
	commChannelStatusLogType_ = rhs.commChannelStatusLogType_;
	commChannelType_ = rhs.commChannelType_;
	commServerMatchType_ = rhs.commServerMatchType_;
	localIP_ = rhs.localIP_;
	localPort_ = rhs.localPort_;
	multicastIP_ = rhs.multicastIP_;
	multicastPort_ = rhs.multicastPort_;
	protocolCfgFile_ = rhs.protocolCfgFile_;
	protocolType_ = rhs.protocolType_;
	remoteID_ = rhs.remoteID_;
	remoteIPs_ = rhs.remoteIPs_;
	remoteParaIndex_ = rhs.remoteParaIndex_;
	remotePorts_ = rhs.remotePorts_;

	DisableCommpointByCloseConnect_ = rhs.DisableCommpointByCloseConnect_;
}

CCommPara & CCommPara::operator = (const CCommPara & rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	bPrintFrame_ = rhs.bPrintFrame_;
	bPrintStatus_ = rhs.bPrintStatus_;
	broadcastIP_ = rhs.broadcastIP_;
	broadcastPort_ = rhs.broadcastPort_;
	commChannelFrameLog_ = rhs.commChannelFrameLog_;
	commChannelFrameLogLimit_ = rhs.commChannelFrameLogLimit_;
	commChannelFrameLogType_ = rhs.commChannelFrameLogType_;
	commChannelStatusLog_ = rhs.commChannelStatusLog_;
	commChannelStatusLogLimit_ = rhs.commChannelStatusLogLimit_;
	commChannelStatusLogType_ = rhs.commChannelStatusLogType_;
	commChannelType_ = rhs.commChannelType_;
	commServerMatchType_ = rhs.commServerMatchType_;
	localIP_ = rhs.localIP_;
	localPort_ = rhs.localPort_;
	multicastIP_ = rhs.multicastIP_;
	multicastPort_ = rhs.multicastPort_;
	protocolCfgFile_ = rhs.protocolCfgFile_;
	protocolType_ = rhs.protocolType_;
	remoteID_ = rhs.remoteID_;
	remoteIPs_ = rhs.remoteIPs_;
	remoteParaIndex_ = rhs.remoteParaIndex_;
	remotePorts_ = rhs.remotePorts_;

	DisableCommpointByCloseConnect_ = rhs.DisableCommpointByCloseConnect_;

	return *this;
}

CCommPara::~CCommPara(void)
{
}

//file name api
std::string CCommPara::getProtocolCfgFileName()
{
	return protocolCfgFile_;
}

int CCommPara::setProtocolCfgFileName(std::string val)
{
	protocolCfgFile_ = val;

	return 0;
}

bool CCommPara::getEnableSpecialProtocolCfg()
{
	return !getProtocolCfgFileName().empty();
}

std::string CCommPara::getCommChannelStatusLogFileName()
{
	return commChannelStatusLog_;
}

int CCommPara::setCommChannelStatusLogFileName(std::string val)
{
	commChannelStatusLog_ = val;

	return 0;
}

bool CCommPara::getEnableChannelStatusLog()
{
	return !getCommChannelStatusLogFileName().empty();
}

std::string CCommPara::getCommChannelFrameLogFileName()
{
	return commChannelFrameLog_;
}

int CCommPara::setCommChannelFrameLogFileName(std::string val)
{
	commChannelFrameLog_ = val;

	return 0;
}

bool CCommPara::getEnableChannelFrameLog()
{
	return !getCommChannelFrameLogFileName().empty();
}

std::string CCommPara::getCommChannelStatusLogType()
{
	return commChannelStatusLogType_;
}

int CCommPara::setCommChannelStatusLogType(std::string val)
{
	commChannelStatusLogType_ = val;

	return 0;
}

std::string CCommPara::getCommChannelStatusLogLimit()
{
	return commChannelStatusLogLimit_;
}

int CCommPara::setCommChannelStatusLogLimit(std::string val)
{
	//	int Limit = atoi(val.c_str());
	commChannelStatusLogLimit_ = val;

	return 0;
}

std::string CCommPara::getCommChannelFrameLogType()
{
	return commChannelFrameLogType_;
}

int CCommPara::setCommChannelFrameLogType(std::string val)
{
	commChannelFrameLogType_ = val;

	return 0;
}

std::string CCommPara::getCommChannelFrameLogLimit()
{
	return commChannelFrameLogLimit_;
}

int CCommPara::setCommChannelFrameLogLimit(std::string val)
{
	commChannelFrameLogLimit_ = val;

	return 0;
}

//para format api
bool CCommPara::AssertIPFormat(std::string val)
{
	std::string regstr = "(1?\\d{1,2}|2[01234]\\d|25[012345])\\.(1?\\d{1,2}|2[01234]\\d|25[012345])\\.(1?\\d{1,2}|2[01234]\\d|25[012345])\\.(1?\\d{1,2}|2[01234]\\d|25[012345])";  
	boost::regex expression(regstr);

	return boost::regex_match(val,expression);
}

bool CCommPara::AssertNetPortFormat(std::string val)
{
	std::string regstr = "\\d{1,5}";
	boost::regex expression(regstr);

	return boost::regex_match(val,expression);
}

bool CCommPara::AssertSerialPortFormat(std::string val)
{
	std::string regstr = "COM\\d{1,3}|(.*?)(ttyS\\d{1,3})|(.*?)(ttyBF\\d{1,3})|(.*?)(UART\\d{1,3})";
	boost::regex expression(regstr,boost::regex::icase);

	return boost::regex_match(val,expression);
}

bool CCommPara::getPrintFrameTerm()
{
	return bPrintFrame_;
}

int CCommPara::setPrintFrameTerm(bool val)
{
	bPrintFrame_ = val;

	return 0;
}

bool CCommPara::getPrintStatusTerm()
{
	return bPrintStatus_;
}

int CCommPara::setPrintStatusTerm(bool val)
{
	bPrintStatus_ = val;

	return 0;
}

std::string CCommPara::getLocalIP()
{
	return localIP_;
}

std::string CCommPara::getLocalPort()
{
	return localPort_;
}

std::string CCommPara::getRemoteID()
{
	return remoteID_;
}

std::string CCommPara::getCommChannelType()
{
	return commChannelType_;
}

std::string CCommPara::getProtocolType()
{
	return protocolType_;
}

int CCommPara::setLocalIP(std::string val)
{
	if (AssertIPFormat(val))
	{
		localIP_ = val;

		return 0;
	}


	return -1;
}

int CCommPara::setLocalPort(std::string val)
{
	if (AssertNetPortFormat(val) || AssertSerialPortFormat(val))
	{
		localPort_ = val;

		return 0;
	}

	return -1;

}

int CCommPara::setRemoteID(std::string val)
{
	remoteID_ = val;

	return 0;
}

int CCommPara::setCommChannelType( std::string val )
{
	commChannelType_ = val;

	return 0;
}

int CCommPara::setProtocolType( std::string val )
{
	protocolType_ = val;

	return 0;
}

int CCommPara::setMatchType( std::string val )
{
	commServerMatchType_ = val;
	return 0;
}

std::string CCommPara::getMatchType()
{
	return commServerMatchType_;
}

int CCommPara::AddRemoteIP(std::string val)
{
	if (AssertIPFormat(val))
	{
		remoteIPs_.push_back(val);

		return 0;
	}

	return -1;
}

int CCommPara::AddRemotePort(std::string val)
{
	if (AssertNetPortFormat(val) || AssertSerialPortFormat(val))
	{
		remotePorts_.push_back(val);

		return 0;
	}

	return -1;
}

void CCommPara::ClearRemoteIP()
{
	remoteIPs_.clear();
}

void CCommPara::ClearRemotePort()
{
	remotePorts_.clear();
}

int CCommPara::getNextRemotePara(std::string & ip,std::string port)
{
	if ((remoteIPs_.size() == 0) || (remotePorts_.size() == 0))
	{
		return -1;
	}

	int num = (remoteIPs_.size() < remotePorts_.size()) ? remoteIPs_.size() : remotePorts_.size();

	if (remoteParaIndex_ < 0 || remoteParaIndex_ >= num)
	{
		remoteParaIndex_ = 0;
	}

	ip = remoteIPs_[remoteParaIndex_];
	port = remotePorts_[remoteParaIndex_];

	remoteParaIndex_ = (++remoteParaIndex_) % num;

	return 0;
}

bool CCommPara::MatchRemoteIP(std::string val)
{
	boost::trim(val);
	if (!AssertIPFormat(val))
	{
		return false;
	}

	for (size_t i=0;i<remoteIPs_.size();i++)
	{
		if (boost::iequals(remoteIPs_[i],val))
		{
			return true;
		}
	}

	return false;
}

int CCommPara::getRemoteIPSum()
{
	return remoteIPs_.size();
}

int CCommPara::getRemotePortSum()
{
	return remotePorts_.size();
}

std::string CCommPara::getRemoteIP(int index)
{
	if (index < 0 || index >= getRemoteIPSum())
	{
		return "";
	}

	return remoteIPs_[index];
}

std::string CCommPara::getRemotePort(int index)
{
	if (index < 0 || index >= getRemotePortSum())
	{
		return "";
	}

	return remotePorts_[index];
}

void CCommPara::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddElem(strCommChannelType,getCommChannelType());
	if (getbDisableCommpointByCloseConnect())
	{
		xml.AddAttrib(strDisableCommpointByCloseConnec,strboolTrue);
	}
	if (!getMatchType().empty())
	{
		xml.AddAttrib(strMatchType,getMatchType());
	}

	xml.AddElem(strProtocolType,getProtocolType());

	if (!getLocalIP().empty())
	{
		xml.AddElem(strLocalIP,getLocalIP());
	}

	if (!getLocalPort().empty())
	{
		xml.AddElem(strLocalPort,getLocalPort());
	}

	if (!getBroadcastIP().empty())
	{
		xml.AddElem(strBroadcastIP,getBroadcastIP());
	}

	if (!getBroadcastPort().empty())
	{
		xml.AddElem(strBroadcastPort,getBroadcastPort());
	}

	if (!getMulticastIP().empty())
	{
		xml.AddElem(strMulticastIP,getMulticastIP());
	}

	if (!getMulticastPort().empty())
	{
		xml.AddElem(strMulticastPort,getMulticastPort());
	}

	for (int i=0;i<getRemoteIPSum();i++)
	{
		if(!remoteIPs_[i].empty())
		{
			xml.AddElem(strRemoteIP,remoteIPs_[i]);
		}
	}

	for (int i=0;i<getRemotePortSum();i++)
	{
		if (!remotePorts_.empty())
		{
			xml.AddElem(strRemotePort,remotePorts_[i]);
		}
	}

	if (getEnableChannelStatusLog())
	{
		xml.AddElem(strChannelStatusLog,getCommChannelStatusLogFileName());
		if (!getCommChannelStatusLogType().empty())
		{
			xml.AddAttrib(strFileType,getCommChannelStatusLogType());
		}
		if (!getCommChannelStatusLogLimit().empty())
		{
			xml.AddAttrib(strLimit,getCommChannelStatusLogLimit());
		}
	}

	if (getEnableChannelFrameLog())
	{
		xml.AddElem(strChannelFrameLog,getCommChannelFrameLogFileName());
		if (!getCommChannelFrameLogType().empty())
		{
			xml.AddAttrib(strFileType,getCommChannelFrameLogType());
		}
		if (!getCommChannelFrameLogLimit().empty())
		{
			xml.AddAttrib(strLimit,getCommChannelFrameLogLimit());
		}
	}

	if (getEnableSpecialProtocolCfg())
	{
		xml.AddElem(strProtocolSpecialCfg,getProtocolCfgFileName());
	}

	if (getPrintFrameTerm())
	{
		xml.AddElem(strPrintFrameTerm,strboolTrue);
	}

	if (getPrintStatusTerm())
	{
		xml.AddElem(strPrintStatusTerm,strboolTrue);
	}
}

int CCommPara::LoadXmlCfg(FileSystem::CMarkup &xml)
{
	xml.ResetMainPos();
	if (xml.FindElem(strCommChannelType))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setCommChannelType(strTmp);

		strTmp = xml.GetAttrib(strMatchType);
		boost::algorithm::trim(strTmp);
		if (!strTmp.empty())
		{
			setMatchType(strTmp);
		}

		strTmp = xml.GetAttrib(strDisableCommpointByCloseConnec);
		boost::algorithm::trim(strTmp);
		if (!strTmp.empty())
		{
			if (boost::iequals(strboolTrue,strTmp))
			{
				setbDisableCommpointByCloseConnect(true);
			}
			else
			{
				setbDisableCommpointByCloseConnect(false);
			}
		}
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(commchannel_para_missing);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strProtocolType))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setProtocolType(strTmp);
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(protocol_para_missing);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strLocalIP))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (AssertIPFormat(strTmp))
		{
			setLocalIP(strTmp);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strLocalPort))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (AssertNetPortFormat(strTmp) || AssertSerialPortFormat(strTmp))
		{
			setLocalPort(strTmp);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strBroadcastIP))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (AssertIPFormat(strTmp))
		{
			setBroadcastIP(strTmp);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strBroadcastPort))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (AssertNetPortFormat(strTmp))
		{
			setBoardcastPort(strTmp);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strMulticastIP))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (AssertIPFormat(strTmp))
		{
			setMulticastIP(strTmp);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strMulticastPort))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (AssertNetPortFormat(strTmp))
		{
			setMulticastPort(strTmp);
		}
	}

	xml.ResetMainPos();
	while (xml.FindElem(strRemotePort))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (AssertNetPortFormat(strTmp))
		{
			AddRemotePort(strTmp);
		}
	}

	xml.ResetMainPos();
	while (xml.FindElem(strRemoteIP))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (AssertIPFormat(strTmp))
		{
			AddRemoteIP(strTmp);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strChannelStatusLog))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setCommChannelStatusLogFileName(strTmp);

		strTmp = xml.GetAttrib(strFileType);
		boost::algorithm::trim(strTmp);
		setCommChannelStatusLogType(strTmp);

		strTmp = xml.GetAttrib(strLimit);
		boost::algorithm::trim(strTmp);
		setCommChannelStatusLogLimit(strTmp);

	}

	xml.ResetMainPos();
	if (xml.FindElem(strChannelFrameLog))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setCommChannelFrameLogFileName(strTmp);

		strTmp = xml.GetAttrib(strFileType);
		boost::algorithm::trim(strTmp);
		setCommChannelFrameLogType(strTmp);

		strTmp = xml.GetAttrib(strLimit);
		boost::algorithm::trim(strTmp);
		setCommChannelFrameLogLimit(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strProtocolSpecialCfg))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		setProtocolCfgFileName(strTmp);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strPrintFrameTerm))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			setPrintFrameTerm(true);
		}
		else
		{
			setPrintFrameTerm(false);
		}
	}

	xml.ResetMainPos();
	if (xml.FindElem(strPrintStatusTerm))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolTrue,strTmp))
		{
			setPrintStatusTerm(true);
		}
		else
		{
			setPrintStatusTerm(false);
		}
	}

	return 0;
}

std::string CCommPara::getBroadcastIP()
{
	return broadcastIP_;
}

std::string CCommPara::getBroadcastPort()
{
	return broadcastPort_;
}

std::string CCommPara::getMulticastIP()
{
	return multicastIP_;
}

std::string CCommPara::getMulticastPort()
{
	return multicastPort_;
}

int CCommPara::setBroadcastIP(std::string val)
{
	broadcastIP_ = val;

	return 0;
}

int CCommPara::setBoardcastPort(std::string val)
{
	broadcastPort_ = val;

	return 0;
}

int CCommPara::setMulticastIP(std::string val)
{
	multicastIP_ = val;

	return 0;
}

int CCommPara::setMulticastPort(std::string val)
{
	multicastPort_ = val;

	return 0;
}

bool CCommPara::getbDisableCommpointByCloseConnect()
{
	return DisableCommpointByCloseConnect_;
}

int CCommPara::setbDisableCommpointByCloseConnect(bool val)
{
	DisableCommpointByCloseConnect_ = val;

	return 0;
}

};//namespace CommInterface
