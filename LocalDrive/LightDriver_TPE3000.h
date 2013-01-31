#pragma once
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/logic/tribool.hpp>

namespace LocalDrive {

typedef unsigned char typeLedStatus;
typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr; 

const typeLedStatus LED_ON = 1;        //LED亮
const typeLedStatus LED_OFF = 2;       //LED灭
const typeLedStatus LED_ON_OFF = 3;    //LED闪烁，先亮再灭
const typeLedStatus LED_OFF_ON = 4;    //LED闪烁，先灭再亮
const typeLedStatus LED_BLING = 5;     //LED闪烁，根据当前状态取反再恢复

class CLightDriver_TPE3000
{
public:
	CLightDriver_TPE3000(boost::asio::io_service & io_service);
	~CLightDriver_TPE3000(void);

	int run();
	int stop();
	int reset(int index,std::vector<typeLedStatus> status);

	int getLedSum();

	int WriteLed(int index,bool OnOrOff);      //bool OnOrOff: true = on; false = off;
	int ReadLed(int index);                    //return int 1 = on; 0 = off; -1 = err;
	int BlingLed(int index,int blingTimes,boost::logic::tribool blingstatus = boost::logic::indeterminate);

private:
	int OpenLed();
	void CloseLed();

	int ResetTimerBlingLed(int index,int blingTimes,boost::logic::tribool blingstatus);
	void handle_timerBlingLed(const boost::system::error_code& error,int index,int blingTimes,boost::logic::tribool blingstatus);

	int ResetTimerReset(int index,std::vector<typeLedStatus> status);
	void handle_timerReset(const boost::system::error_code& error,int index,std::vector<typeLedStatus> status);

private:
	enum
	{
		MAX_LED_NUM = 8,
		BLING_INTERVAL = 200,//闪烁时间间隔 单位：毫秒
	};

	int LedHandle_;
	std::vector<timer_ptr> led_timers_;
};

};//namespace LocalDrive
