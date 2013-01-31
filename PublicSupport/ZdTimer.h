#pragma once

#include <boost/asio/deadline_timer.hpp>

namespace PublicSupport {

class CZdTimer
	:public boost::asio::deadline_timer
{
public:
	CZdTimer(boost::asio::io_service & io_service);
	~CZdTimer(void);

	//每整数倍xx小时、分钟、秒时间点
	boost::posix_time::time_duration expires_zd_hour(boost::posix_time::ptime cur_lt,size_t hour_val);   //每hour_val小时，从0时开始算
	boost::posix_time::time_duration expires_zd_minutes(boost::posix_time::ptime cur_lt,size_t min_val); //每min_val分钟，从0分开始计算
	boost::posix_time::time_duration expires_zd_seconds(boost::posix_time::ptime cur_lt,size_t sec_val); //每sec_val秒，从0秒开始计算

	//每自然单位时间的某时刻
	boost::posix_time::time_duration expires_time_everyday(boost::posix_time::ptime cur_lt,boost::posix_time::time_duration time_val); //每天的某时某分某秒
	boost::posix_time::time_duration expires_time_everyhour(boost::posix_time::ptime cur_lt,boost::posix_time::time_duration time_val); //每小时的某分某秒
	boost::posix_time::time_duration expires_time_everyminute(boost::posix_time::ptime cur_lt,boost::posix_time::time_duration time_val); //每分钟的某秒

	boost::posix_time::time_duration expires_time_everyyear(boost::posix_time::ptime cur_lt,boost::posix_time::ptime time_val); //每年的某个时刻

};

};//namespace PublicSupport

