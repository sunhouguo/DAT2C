#pragma once
#include <boost/asio.hpp>
#include "WatchDog.h"

namespace LocalDrive {

class CWatchDog_BF518 :
	public CWatchDog
{
public:
	CWatchDog_BF518(boost::asio::io_service & io_service,bool  bEnableLog);
	virtual ~CWatchDog_BF518(void);

	int run();
	int stop();
	int reset();

private:
	int OpenWatchDog();
	void CloseWatchDog();
	int OpenRunLight();
	void CloseRunLight();
	void handle_timerFeedWatchDog(const boost::system::error_code& error);
	void handle_timerRunLight(const boost::system::error_code& error);
	void ResetTimerFeedWatchDog(bool bContinue,unsigned short timeVal);
	void ResetTimerRunLight(bool bContinue,unsigned short timeVal);

private:
	enum
	{
		DefaultWatchDogTimeOut = 180, //默认看门狗超时时间 单位：秒
		DefaultWatchDogTimeFeed = 5, //默认喂狗时间 单位：秒
		DefaultRunSingalTime = 1
	};

	int WatchDogHandle_;
	int WatchDogTimeOut_;
	int WatchDogTimeFeed_;

	bool RunLightFlag_;
	int RunLightHandle_;
	int RunLightTime_;
	unsigned long feedCount_;
	boost::asio::io_service & io_service_;
	boost::asio::deadline_timer timerFeedWatchDog_;
	boost::asio::deadline_timer timerRunLight_;
};

}; //namespace LocalDrive

