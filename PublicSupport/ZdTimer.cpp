#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/exception/all.hpp>
#include "ZdTimer.h"
#include "../PublicSupport/Dat2cTypeDef.h"

namespace PublicSupport {

using namespace boost::posix_time;

CZdTimer::CZdTimer(boost::asio::io_service & io_service)
	:boost::asio::deadline_timer(io_service)
{
}

CZdTimer::~CZdTimer(void)
{
}

//************************************
// Method:    expires_zd_hour
// FullName:  PublicSupport::CZdTimer::expires_zd_hour
// Access:    public 
// Returns:   boost::posix_time::time_duration
// Qualifier: 获得当前时刻离下一个整点小时时刻的时间段数值
// Parameter: ptime cur_lt 指定的当前时刻
// Parameter: size_t hour_val 整点间隔公分数
//************************************
time_duration CZdTimer::expires_zd_hour(ptime cur_lt,size_t hour_val)
{
	if (hour_val <= 0 || hour_val >= 24)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	size_t next_trig_hour;
	if (24 - cur_lt.time_of_day().hours() >= hour_val)
	{
		next_trig_hour = hour_val - (cur_lt.time_of_day().hours() % hour_val) - 1;
	}
	else
	{
		next_trig_hour = hour_val + 24 - cur_lt.time_of_day().hours() -1;
	}

	size_t next_trig_min = 60 - cur_lt.time_of_day().minutes() -1;
	size_t next_trig_sec = 60 - cur_lt.time_of_day().seconds();

	return time_duration(next_trig_hour,next_trig_min,next_trig_sec);
}

//************************************
// Method:    expires_zd_minutes
// FullName:  PublicSupport::CZdTimer::expires_zd_minutes
// Access:    public 
// Returns:   boost::posix_time::time_duration
// Qualifier: 获得当前时刻离下一个整点分钟时刻的时间段数值
// Parameter: boost::posix_time::ptime cur_lt 指定的当前时刻
// Parameter: size_t min_val 整点间隔公分数
//************************************
time_duration CZdTimer::expires_zd_minutes(boost::posix_time::ptime cur_lt,size_t min_val)
{
	if (min_val <= 0 || min_val >= 60)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	size_t next_trig_min;
	if(60 - cur_lt.time_of_day().minutes() >= min_val)
	{
		next_trig_min = min_val - (cur_lt.time_of_day().minutes() % min_val) - 1;
	}
	else
	{
		 next_trig_min = min_val + 60 - cur_lt.time_of_day().minutes() - 1;
	}
	
	size_t next_trig_sec = 60 - cur_lt.time_of_day().seconds();

	return time_duration(0,next_trig_min,next_trig_sec);
}

//************************************
// Method:    expires_zd_seconds
// FullName:  PublicSupport::CZdTimer::expires_zd_seconds
// Access:    public 
// Returns:   boost::posix_time::time_duration
// Qualifier: 获得当前时刻离下一个整点小时时刻的时间段数值
// Parameter: boost::posix_time::ptime cur_lt 指定的当前时刻
// Parameter: size_t sec_val 整点间隔公分数
//************************************
time_duration CZdTimer::expires_zd_seconds(boost::posix_time::ptime cur_lt,size_t sec_val)
{
	if (sec_val <= 0 || sec_val >= 60)
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	size_t next_trig_sec;
	if(60 - cur_lt.time_of_day().seconds() >= sec_val)
	{
		next_trig_sec = sec_val - (cur_lt.time_of_day().seconds() % sec_val);
	}
	else
	{
		next_trig_sec = sec_val + 60 - cur_lt.time_of_day().seconds();
	}

	return time_duration(0,0,next_trig_sec);
}

//************************************
// Method:    expires_time_everyday
// FullName:  PublicSupport::CZdTimer::expires_time_everyday
// Access:    public 
// Returns:   boost::posix_time::time_duration
// Qualifier: 每天的某时刻
// Parameter: boost::posix_time::ptime cur_lt
// Parameter: boost::posix_time::time_duration time_val
//************************************
time_duration CZdTimer::expires_time_everyday(boost::posix_time::ptime cur_lt,boost::posix_time::time_duration time_val)
{
	if (time_val >= time_duration(24,0,0) || time_val < time_duration(0,0,0))
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	if (cur_lt.time_of_day() <= time_val)
	{
		return time_val - cur_lt.time_of_day();
	}
	else
	{
		return time_val + time_duration(24,0,0) - cur_lt.time_of_day();
	}
}

//************************************
// Method:    expires_time_everyhour
// FullName:  PublicSupport::CZdTimer::expires_time_everyhour
// Access:    public 
// Returns:   boost::posix_time::time_duration
// Qualifier: 每小时的某时刻
// Parameter: boost::posix_time::ptime cur_lt
// Parameter: boost::posix_time::time_duration time_val
//************************************
time_duration CZdTimer::expires_time_everyhour(boost::posix_time::ptime cur_lt,boost::posix_time::time_duration time_val)
{
	if (time_val >= time_duration(0,60,0) || time_val < time_duration(0,0,0))
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	time_duration off_hours = (cur_lt.time_of_day() - hours(cur_lt.time_of_day().hours()));
	if (off_hours <= time_val)
	{
		return time_val - off_hours;
	}
	else
	{
		return time_val + time_duration(0,60,0) - off_hours;
	}
}

//************************************
// Method:    expires_time_everyminute
// FullName:  PublicSupport::CZdTimer::expires_time_everyminute
// Access:    public 
// Returns:   boost::posix_time::time_duration
// Qualifier: 每分钟的某时刻
// Parameter: boost::posix_time::ptime cur_lt
// Parameter: boost::posix_time::time_duration time_val
//************************************
boost::posix_time::time_duration CZdTimer::expires_time_everyminute(boost::posix_time::ptime cur_lt,boost::posix_time::time_duration time_val)
{
	if (time_val >= time_duration(0,0,60) || time_val < time_duration(0,0,0))
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(IndexOutOfMemoryErr);
	}

	time_duration off_hours_mins = cur_lt.time_of_day() - hours(cur_lt.time_of_day().hours()) - minutes(cur_lt.time_of_day().minutes());
	if (off_hours_mins <= time_val)
	{
		return time_val - off_hours_mins;
	}
	else
	{
		return time_val + time_duration(0,0,60) - off_hours_mins;
	}
}

//************************************
// Method:    expires_time_everyyear
// FullName:  PublicSupport::CZdTimer::expires_time_everyyear
// Access:    public 
// Returns:   boost::posix_time::time_duration
// Qualifier: 每年的某个时刻启动
// Parameter: boost::posix_time::ptime cur_lt
// Parameter: boost::posix_time::ptime time_val
//************************************
boost::posix_time::time_duration CZdTimer::expires_time_everyyear(boost::posix_time::ptime cur_lt,boost::posix_time::ptime time_val)
{
	ptime lt_val(boost::gregorian::date(cur_lt.date().year(),time_val.date().month(),time_val.date().day()),time_val.time_of_day());

	if (lt_val < cur_lt)
	{
		lt_val += boost::gregorian::years(1);
	}

	return lt_val - cur_lt;
}

};//namespace PublicSupport
