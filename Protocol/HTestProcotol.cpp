#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "HTestprocotol.h"
#include "../FileSystem/Markup.h"
#include "../PublicSupport/Dat2cPublicAPI.h"
#include "../PublicSupport/Dat2cTypeDef.h"

namespace Protocol {

	const std::string strDefaultCfg = "HTestCfg.xml";
	size_t CTestHProcotol::HTestObjectCounter_ = 1;


CTestHProcotol::CTestHProcotol(boost::asio::io_service & io_service)
:CProtocol(io_service)
{
	SynCharNum_ = 1;
	ProtocolObjectIndex_ = HTestObjectCounter_++;

	timeOutHeartFrame_=DEFAULT_timeOutHeartFrame;
	AssembleCount_=1;

	InitDefaultTimer(io_service);

	LoadXmlCfg(strDefaultCfg);
}

CTestHProcotol::~CTestHProcotol(void)
{
	HTestObjectCounter_--;
}

int CTestHProcotol::InitProtocol()
{

	CProtocol::InitProtocol();

	if(getCommPointSum() > 0)
	{
		share_commpoint_ptr nextPoint = getFirstCommPoint();
		if (nextPoint)
		{
			AddSendCmdVal(START_ACT,START_ACT_PRIORITY,nextPoint);
		}
	}

	AddStatusLogWithSynT("HTest规约的通道打开成功。\n");

	return 0;
}

void CTestHProcotol::UninitProtocol()
{
	CProtocol::UninitProtocol();

	AddStatusLogWithSynT("HTest规约的通道关闭成功。\n");
}

int CTestHProcotol::CheckFrameHead(unsigned char * buf,size_t & exceptedBytes)
{
	exceptedBytes = getFrameBufLeft();

	return 0;
}

int CTestHProcotol::CheckFrameTail(unsigned char * buf,size_t exceptedBytes)
{
	return 0;
}

int CTestHProcotol::ParseFrameBody(unsigned char * buf,size_t exceptedBytes)
{
	unsigned short count=BufToVal(&buf[1],1);
	if(count==AssembleCount_)
	{
	boost::posix_time::ptime now_time = boost::posix_time::microsec_clock::local_time();
	if(now_time>begin_time_)
	{
		std::string strTime = to_simple_string(begin_time_);
		std::ostringstream ostr;
		std::cout<<"begin_time_为："<<strTime<<std::endl;//ostr
//		AddStatusLogWithSynT(ostr.str());

		strTime = to_simple_string(now_time);
		std::cout<<"now_time为："<<strTime<<std::endl;
//		AddStatusLogWithSynT(ostr.str());

		strTime = to_simple_string(now_time-begin_time_);
		std::cout<<"时差为："<<strTime<<std::endl;
//		AddStatusLogWithSynT(ostr.str());
	    
		AssembleCount_++;
	}
   }
	return 0;
}

int CTestHProcotol::AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	//buf[count++] = 0xff;
	//buf[count++] = 0x00;
	
	return count - bufIndex;
}

int CTestHProcotol::AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	int bytesAssemble = 0;
	
	switch (cmd.getCmdType())
	{
	case START_ACT:
		bytesAssemble = AssembleStartDTAct(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			ResetTimerHeartFrame(cmd.getCommPoint(),true);
			setWaitingForAnswer(cmd.getCommPoint());
		}
		break;

	case TEST_ACT:
		bytesAssemble = AssembleTestFRAct(bufIndex,buf);
		if (bytesAssemble > 0)
		{
			setWaitingForAnswer(cmd.getCommPoint());
			ResetTimerHeartFrame(cmd.getCommPoint(),true);
		}
		break;

	}
	
	begin_time_ = boost::posix_time::microsec_clock::local_time();
	return bytesAssemble;
}

int CTestHProcotol::AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd)
{
	size_t count = bufIndex;

	//buf[count++] = 0x00;
	//buf[count++] = 0x0f;
	
	return count - bufIndex;
}

int CTestHProcotol::AssembleStartDTAct(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0xff;
	buf[count++] = 0;
	buf[count++] = 0;
	buf[count++] = 0x22;

	return count - bufIndex;
}

int CTestHProcotol::AssembleTestFRAct(size_t bufIndex, unsigned char * buf)
{
	size_t count = bufIndex;

	buf[count++] = 0xff;
	buf[count++] = (AssembleCount_)&0xff;
	buf[count++] = (AssembleCount_)&0xff;
	buf[count++] = 0x22;

	return count - bufIndex;
}
	
int CTestHProcotol::LoadXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{
		return -1;
	}

	xml.ResetMainPos();
	xml.FindElem();  //root strProtocolRoot
	xml.IntoElem();  //enter strProtocolRoot

	CProtocol::LoadXmlCfg(xml);

	xml.OutOfElem(); //out strProtocolRoot

	return 0;
}

void CTestHProcotol::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strProtocolXsl);

	xml.AddElem(strProtocolRoot);
	xml.IntoElem();

	CProtocol::SaveXmlCfg(xml);

	xml.OutOfElem();

	xml.Save(filename);
}

void CTestHProcotol::InitDefaultTimer(boost::asio::io_service & io_service)
{
	using namespace boost::asio;
	using namespace boost::posix_time;

	timerHeartFrame_.reset(new deadline_timer(io_service,seconds(timeOutHeartFrame_)));
	AddTimer(timerHeartFrame_);
}



void CTestHProcotol::handle_timerHeartFrame(const boost::system::error_code& error,share_commpoint_ptr point)
{
	if (!error)
	{
		{
			AddSendCmdVal(TEST_ACT,TEST_ACT_PRIORITY,point);
		}
	}
}

void CTestHProcotol::ResetTimerHeartFrame(share_commpoint_ptr point,bool bContinue /*= true*/,unsigned short val/* = 0*/)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerHeartFrame_->expires_from_now(boost::posix_time::seconds(timeOutHeartFrame_));
		}
		else
		{
			timerHeartFrame_->expires_from_now(boost::posix_time::seconds(val));
		}

		timerHeartFrame_->async_wait(boost::bind(&CTestHProcotol::handle_timerHeartFrame,this,boost::asio::placeholders::error,point));
	}
	else
	{
		timerHeartFrame_->cancel();
	}
}


}; //namespace Protocol 
