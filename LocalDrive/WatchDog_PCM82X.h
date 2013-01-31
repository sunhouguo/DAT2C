#pragma once
#include <boost/asio.hpp>
#include "WatchDog.h"
//#include "tick_count_timer.h"

//namespace FileSystem
//{
//	class CLog;
//}

namespace LocalDrive {

class CWatchDog_PCM82X
	:public CWatchDog
{
public:
	CWatchDog_PCM82X(boost::asio::io_service & io_service,bool  bEnableLog);
	virtual ~CWatchDog_PCM82X(void);
    int run();
    int stop();
	int reset();
	

private:
	int OpenWatchDog();
	void CloseWatchDog();
	void handle_timerFeedWatchDog(const boost::system::error_code& error);
	//int AddLogWithSynT(std::string strVal);
	void ResetTimerFeedWatchDog(bool bContinue,unsigned short timeVal);

private:
	enum
	{
		DefaultWatchDogTimeOut = 180, //默认看门狗超时时间 单位：秒
		DefaultWatchDogTimeFeed = 120 //默认喂狗时间 单位：秒
	};

	int WatchDogHandle_;
	int WatchDogTimeOut_;
	int WatchDogTimeFeed_;
    unsigned long feedCount_;
	boost::asio::io_service & io_service_;
	boost::asio::deadline_timer timerFeedWatchDog_;
	//tick_count_timer timerFeedWatchDog_;
	//boost::scoped_ptr<FileSystem::CLog> Log_;
};

}//namespace LocalDrive

