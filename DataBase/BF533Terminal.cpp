#include "BF533Terminal.h"
#include "../Protocol/CmdQueue.h"
#include "../CommInterface/CommInterface.h"

namespace DataBase {

#define strYcISendByTime "YcISendByTime"
#define strDataBaseNum   "DataBaseNum"
#define strDstNode       "DstNode"
#define strDstNo         "DstNo"
#define strYcISendFeq     "YcISendFeq"

CBF533Terminal * CBF533Terminal::instance_ = NULL;

CBF533Terminal::CBF533Terminal(boost::asio::io_service & io_service,CSubStation & sub)
							:CTerminal(io_service,sub)
{
	terminalType_ = BF533Terminal;
	bCommActive_ = true;
	YcISendFlage_ = false;
	bCommActiveBackup_ = bCommActive_;
	InitDefaultTimer(io_service_);
}

CBF533Terminal::~CBF533Terminal(void)
{
}

CBF533Terminal * CBF533Terminal::getMyInstance(boost::asio::io_service & io_service,CSubStation & sub)
{
	if (!instance_)
	{
		instance_ = new CBF533Terminal(io_service,sub);
	}

	return instance_;
}

void CBF533Terminal::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	CTerminal::SaveXmlCfg(xml);

	//装置一级的参数应该在此读取，而不应该另外再有一个配置文件。
	//......

	if (YcISendFlage_)
	{
		xml.AddElem(strYcISendByTime);
		xml.AddAttrib(strDataBaseNum,getYcISendNum());
		xml.AddAttrib(strYcISendFeq,timeOutYcISendTime_);
		using namespace std;
		int Index = 0;
		for (vector<int>::iterator it = TerminalYcINum_.begin();it != TerminalYcINum_.end();it++)
		{
			xml.AddChildElem(strDstNode);
			xml.AddChildAttrib(strDstNo,Index ++);
			xml.SetData(* it);
		}
	}
}

int CBF533Terminal::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	int ret = CTerminal::LoadXmlCfg(xml);
	//装置一级的参数应该在此读取，而不应该另外再有一个配置文件。
	//......

	xml.ResetMainPos();
	if (xml.FindElem(strYcISendByTime))
	{
		YcISendFlage_ = true;
		try
		{
			std::string strTmp = xml.GetAttrib(strDataBaseNum);
			boost::algorithm::trim(strTmp);
			setYcISendNum(boost::lexical_cast<int>(strTmp));

			std::string strTmp1 = xml.GetAttrib(strYcISendFeq);
			boost::algorithm::trim(strTmp1);
			setYcISendFeq(boost::lexical_cast<int>(strTmp1));

			xml.IntoElem();
			xml.ResetMainPos();
			while(xml.FindElem(strDstNode))
			{
				std::string strTmp = xml.GetData();
				boost::trim(strTmp);
				int DataNo = boost::lexical_cast<int>(strTmp);

				try
				{
					TerminalYcINum_.push_back(DataNo);
				}
				catch(...)
				{
					std::ostringstream ostr;
					ostr<<"读取遥测表错误！"<<std::endl;
					throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
				}
			}
			xml.OutOfElem();
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<"非法的参数:"<<e.what();
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}

		if (YcISendFlage_)
		{
			ResetTimerYcISend(true,10);
		}
	}

	return ret;
}

void CBF533Terminal::InitDefaultTimer(boost::asio::io_service & io_service)
{
	InitDefaultTimeOut();

	using namespace boost::asio;
	using namespace boost::posix_time;

	timerYcISend_.reset(new boost::asio::deadline_timer(io_service_));//reset(new deadline_timer(io_service,seconds(timeOutYcISendTime_)));
}

void CBF533Terminal::InitDefaultTimeOut(void)
{
	timeOutYcISendTime_ = DEFAULT_timeOutYcISend;
}

void CBF533Terminal::ResetTimerYcISend(bool bContinue,unsigned short val)
{
	if (bContinue)
	{
		if (val == 0)
		{
			timerYcISend_->expires_from_now(boost::posix_time::seconds(timeOutYcISendTime_));
		}
		else
		{
			timerYcISend_->expires_from_now(boost::posix_time::seconds(val));
		}
		timerYcISend_->async_wait(boost::bind(&CBF533Terminal::handle_timerYcISend,this,boost::asio::placeholders::error));
	}
	else
	{
		timerYcISend_->cancel();
	}
}

void CBF533Terminal::handle_timerYcISend(const boost::system::error_code& error)
{
	//std::cout<<"定时器timerYcISend_时间到"<<std::endl;
	TrigYcSendByTimeEvent(getYcISendNum());//通知对上规约发送数据
	ResetTimerYcISend(true);//再启动定时器
}


int CBF533Terminal::getTerminalYcINum(int tIndex)
{
	using namespace std;
	int Index = 0;
	int ret = -1;
	for (vector<int>::iterator it = TerminalYcINum_.begin();it != TerminalYcINum_.end();it++)
	{
		if (Index == tIndex)
		{
			ret = * it;
            break;
		}
		Index ++;
	}

	return ret;
}

int CBF533Terminal::AddGeneralCmd(Protocol::CCmd cmdVal)
{
	if(getCommActive())
	{
		cmdVal.setCommPoint(getSelfPtr());

		return AddCmdVal(cmdVal);
	}

	return -1;
}

int CBF533Terminal::ActiveBattery(bool val)
{
	if(getCommActive())
	{
		int ret = 0;

		if (val)
		{
			ret = AddCmdVal(Protocol::BATTERY_ACTIVE,Protocol::BATTERY_ACTIVE_PRIORITY,getSelfPtr());
		}
		else
		{
			ret = AddCmdVal(Protocol::BATTERY_ACTIVE_OVER,Protocol::BATTERY_ACTIVE_OVER_PRIORITY,getSelfPtr());
		}

		return ret;
	}

	return -1;
}

//int CBF533Terminal::SynTime(void)
//{
//	if (getCommActive())
//	{
//		return AddCmdVal(Protocol::SYN_TIME_ACT,Protocol::SYN_TIME_ACT_PRIORITY,getSelfPtr());
//	}
//
//	return -1;
//	
//}

void CBF533Terminal::setYcISendNum(int val)
{
	YcISendNum_ = val;
}

void CBF533Terminal::setYcISendFeq(int val)
{
	timeOutYcISendTime_ = val;
}

int  CBF533Terminal::getYcISendNum(void)
{
	return YcISendNum_;
}

int CBF533Terminal::Reconnect()
{
	if (commPtr_)
	{
		return commPtr_->RetryConnect();
	}

	return -1;
}

}; //namespace DataBase 

