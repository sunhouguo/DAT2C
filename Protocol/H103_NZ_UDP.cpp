#include <boost/bind.hpp>
//#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "H103_NZ_UDP.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../DataBase/Terminal.h"

namespace Protocol {

const std::string strDefaultCfg = "H103_NZ_UDPCfg.xml";
size_t CH103_NZ_UDP::H103_NZ_UDPObjectCounter_ = 1;

const std::string strSendBroadCast = "SendBroadCast";

CH103_NZ_UDP::CH103_NZ_UDP(boost::asio::io_service & io_service)
								:CProtocol(io_service)
{
	bActiveRepeatFrame_ = true;
	SynCharNum_ = 5;
	bool bSendBroadCast_ = true;

	InitObjectIndex();
	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);
}

CH103_NZ_UDP::~CH103_NZ_UDP(void)
{
	H103_NZ_UDPObjectCounter_--;
}

int CH103_NZ_UDP::LoadXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{

		//std::cout<<"导入CH103_NZ_UDPCfg.xml配置文件"<<filename<<"失败！"<<std::endl;

		return -1;
	}

	xml.ResetMainPos();
	xml.FindElem();  //root strProtocolRoot
	xml.IntoElem();  //enter strProtocolRoot

	CProtocol::LoadXmlCfg(xml);

	xml.ResetMainPos();
	if (xml.FindElem(strSendBroadCast))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::iequals(strboolFalse,strTmp))
		{
			bSendBroadCast_ = false;
		}
		else
		{
			bSendBroadCast_ = true;
		}
	}

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CH103_NZ_UDP::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();   //enter strProtocolRoot

	CProtocol::SaveXmlCfg(xml);

	if (bSendBroadCast_)
	{
		xml.AddElem(strSendBroadCast,strboolTrue);
	}
	else
	{
		xml.AddElem(strSendBroadCast,strboolFalse);
	}

	xml.OutOfElem();  //out strProtocolRoot

	xml.Save(filename); 
}

int CH103_NZ_UDP::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	exceptedBytes = getFrameBufLeft();
	return 0;
}

int CH103_NZ_UDP::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

int CH103_NZ_UDP::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	//return terminalIndex;
	return 0;
}

int CH103_NZ_UDP::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	buf[count++] = 0xff;
	
	return count - bufIndex;
}

int CH103_NZ_UDP::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	int bytesAssemble = 0;

	share_terminal_ptr terminalPtr;

	if (cmd.getCommPoint())
	{
		if (cmd.getCommPoint()->getCommPointType() == TERMINAL_NODE)
		{
			terminalPtr = boost::dynamic_pointer_cast<DataBase::CTerminal>(cmd.getCommPoint());
		}
	}

	if (!terminalPtr)
	{
		std::ostringstream ostr;
		ostr<<"H103_NZ_UDP规约不能从发送命令中获得terminal ptr，cmdtype = "<<cmd.getCmdType()<<"，这个命令将不会被发送。"<<std::endl;
		AddStatusLogWithSynT(ostr.str());

		return -1;
	}
	
	switch (cmd.getCmdType())
	{
		case QUERY_DATA:
			bytesAssemble = AssembleUDP(bufIndex,buf,terminalPtr);
			if(bytesAssemble > 0)
			{
				setBroadCastSend(bSendBroadCast_);
				ResetTimerUDP(terminalPtr);
			}
			break;
			
		default:
			break;
	}
	
	return bytesAssemble;
}

int CH103_NZ_UDP::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;
	
	return count - bufIndex;
}

int CH103_NZ_UDP::InitProtocol()
{
	CProtocol::InitProtocol();

	if(getCommPointSum() > 0)
	{
		share_commpoint_ptr nextPoint = getFirstCommPoint();
		if (nextPoint)
		{
			AddSendCmdVal(QUERY_DATA,QUERY_DATA_PRIORITY,nextPoint);
		}
	}

	AddStatusLogWithSynT("H103_NZ_UDP规约的通道打开成功。\n");

	return 0;
}

void CH103_NZ_UDP::UninitProtocol()
{
	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("H103_NZ_UDP规约的通道关闭成功。\n");
}

void CH103_NZ_UDP::InitObjectIndex()
{
	ProtocolObjectIndex_ = H103_NZ_UDPObjectCounter_++;
}

void CH103_NZ_UDP::InitDefaultTimeOut()
{
	timeOutUDP_ = DEFAULT_timeOutUDP;
}

void CH103_NZ_UDP::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerUDP_.reset(new deadline_timer(io_service,seconds(timeOutUDP_)));
	AddTimer(timerUDP_);
}

void CH103_NZ_UDP::handle_timerUDP(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		share_commpoint_ptr val = getNextCommPoint(TERMINAL_NODE,boost::logic::indeterminate,point);
		if (val)
		{
			AddSendCmdVal(QUERY_DATA,QUERY_DATA_PRIORITY,val);
		}
	}
}


void CH103_NZ_UDP::ResetTimerUDP(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerUDP_->expires_from_now(boost::posix_time::seconds(timeOutUDP_));
		}
		else
		{
			timerUDP_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerUDP_->async_wait(boost::bind(&CH103_NZ_UDP::handle_timerUDP,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerUDP_->cancel();
	}
}

int CH103_NZ_UDP::setTimeOutUDP(unsigned short val)
{
	if (val <= 0 || val > 60) 
	{
		return -1;
	}

	timeOutUDP_ = val;

	return 0;
}

int CH103_NZ_UDP::AssembleUDP(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr)
{
	size_t count = bufIndex;
	
	for (int i=0;i<40;i++)
	{
		buf[count++] = 0x00;
	}
	
	return count - bufIndex;
}


}; //namespace Protocol 
