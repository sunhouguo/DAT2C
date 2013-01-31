#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "BatteryActive.h"
#include "../FileSystem/Markup.h"
#include "../DataBase/SubStation.h"
#include "../DataBase/BF533Terminal.h"


namespace LocalDrive {

using namespace boost::posix_time;

#define strBatteryCfg "BatteryCfg"
#define strDateNode   "DateNode"

time_duration zero_time_duration = time_duration(0,0,0);
const int hours_a_year = 9000;

CBatteryActive::CBatteryActive(boost::asio::io_service & io_service,std::string strCfg,DataBase::CSubStation & sub)
				:sub_(sub),
				zdtimer_(io_service)
{
    LoadXml(strCfg);
}


CBatteryActive::~CBatteryActive(void)
{
}

int CBatteryActive::run()
{
	time_duration min_time = getMinTimeFromNow();

	if (min_time > zero_time_duration)
	{
		ResetTimerBatteryActive(true,min_time);
		
		return 0;
	}

	return -1;
}

int CBatteryActive::stop()
{
	ResetTimerBatteryActive(false,time_duration());

	return 0;
}

int CBatteryActive::reset()
{
	stop();

	return run();
}

int CBatteryActive::LoadXml(std::string strCfg)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(strCfg))
	{
		return -1;
	}

	xml.ResetMainPos();
	if (xml.FindElem(strBatteryCfg))
	{
		xml.IntoElem();
		
		while(xml.FindElem(strDateNode))
		{
			std::string strTmp = xml.GetData();
			boost::trim(strTmp);

			try
			{
				ptime t(from_iso_string(strTmp));
				times_.push_back(t);
			}
			catch(...)
			{
				std::cerr<<"电池活化抛出异常！"<<std::endl;
			}
			
		}

		xml.OutOfElem();
	}

	return 0;
}

void CBatteryActive::handle_timerBatteryActive(const boost::system::error_code& error)
{
    ActiveBattery(true);

	ResetTimerBatteryActive(true,getMinTimeFromNow());
}

void CBatteryActive::ResetTimerBatteryActive(bool bContinue,boost::posix_time::time_duration timeVal)
{
	if (bContinue)
	{
		if (timeVal > zero_time_duration)
		{
			zdtimer_.expires_from_now(timeVal);
			zdtimer_.async_wait(boost::bind(&CBatteryActive::handle_timerBatteryActive,this,boost::asio::placeholders::error));
		}
	}
	else
	{
		zdtimer_.cancel();
	}
}

boost::posix_time::time_duration CBatteryActive::getMinTimeFromNow()
{
	using namespace std;

	time_duration min_tmp = time_duration(hours_a_year,0,0);
	
	ptime cur_lt = boost::posix_time::microsec_clock::local_time();
	for (vector<ptime>::iterator it = times_.begin();it != times_.end();it++)
	{
		time_duration td_dif = zdtimer_.expires_time_everyyear(cur_lt,*it);
		if (td_dif > zero_time_duration && td_dif < min_tmp)
		{
				min_tmp = td_dif;
		}
	}

	if (min_tmp == time_duration(hours_a_year,0,0))
	{
		return zero_time_duration;
	}
	
	return min_tmp;
}

int CBatteryActive::ActiveBattery(bool val)
{
	DataBase::share_bf533_ptr bf533 = sub_.getBf533Terminal();

	if (bf533)
	{
		return bf533->ActiveBattery(val);
	}

	return -1;
}

};//namespace LocalDrive
