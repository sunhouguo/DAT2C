#include <iostream>
#include <boost/bind.hpp>
#include "WatchDog_TPE3000.h"

#if defined(_TPE3000_)
#include <linux/watchdog.h>
#endif

namespace LocalDrive {

CWatchDog_TPE3000::CWatchDog_TPE3000(boost::asio::io_service & io_service,bool  bEnableLog)
									:CWatchDog(bEnableLog),
									io_service_(io_service),
									timerFeedWatchDog_(io_service)
{
	feedCount_ = 0;
	WatchDogHandle_ = -1;
	WatchDogTimeOut_ = DefaultWatchDogTimeOut;
	WatchDogTimeFeed_ = DefaultWatchDogTimeFeed;
}

CWatchDog_TPE3000::~CWatchDog_TPE3000(void)
{
}

//int CWatchDog_TPE3000::AddLogWithSynT(std::string strVal)
//{
//	if (Log_)
//	{
//		return Log_->AddRecordWithSynT(strVal);
//	}
//
//	return -1;
//}

int CWatchDog_TPE3000::run()
{
	return OpenWatchDog();
}

int CWatchDog_TPE3000::stop()
{
	CloseWatchDog();

	return 0;
}

int CWatchDog_TPE3000::reset()
{
	ResetTimerFeedWatchDog(true,0);

	return 0;
}

int CWatchDog_TPE3000::OpenWatchDog()
{
#if defined(_TPE3000_)

	WatchDogHandle_ = open("/dev/watchdog",O_WRONLY);
	if(WatchDogHandle_ < 0)
	{
		std::ostringstream ostr;
		ostr<<"TPE3000 watchdog device open fail"<<std::endl;
		AddLogWithSynT(ostr.str());
		std::cerr<<ostr.str();
		
		return WatchDogHandle_;
	}

	if(ioctl(WatchDogHandle_,WDIOC_SETTIMEOUT,&WatchDogTimeOut_) >= 0)
	{
		int op = WDIOS_ENABLECARD;
		if(ioctl(WatchDogHandle_, WDIOC_SETOPTIONS, &op) >= 0)
		{
			ResetTimerFeedWatchDog(true,WatchDogTimeFeed_);

			std::ostringstream ostr;
			ostr<<"TPE3000 start dog"<<std::endl;
			AddLogWithSynT(ostr.str());
		}
	}

	return 0;

#endif //defined(_TPE3000_)

	return -1;
}

void CWatchDog_TPE3000::CloseWatchDog()
{
	ResetTimerFeedWatchDog(false,WatchDogTimeFeed_);

#if defined(_TPE3000_)

	if(WatchDogHandle_ >= 0)
	{
		int op = WDIOS_DISABLECARD;
		ioctl(WatchDogHandle_, WDIOC_SETOPTIONS, &op);
		close(WatchDogHandle_);
		WatchDogHandle_ = -1;

		std::ostringstream ostr;
		ostr<<"TPE3000 close dog"<<std::endl;
		AddLogWithSynT(ostr.str());
	}

#endif //defined(_TPE3000_)	

}

void CWatchDog_TPE3000::handle_timerFeedWatchDog(const boost::system::error_code& error)
{
	if(!error)
	{
#if defined(_TPE3000_)

		if (WatchDogHandle_ >= 0)
		{
			std::ostringstream ostr;
			ostr<<"TPE3000 feed dog on the "<<feedCount_++<<" times."<<std::endl;
			AddLogWithSynT(ostr.str());

			//int op = WDIOC_KEEPALIVE;
			//ioctl(WatchDogHandle_, WDIOC_SETOPTIONS, &op);

			int op = 0;
			ioctl(WatchDogHandle_, WDIOC_KEEPALIVE, &op);

			ResetTimerFeedWatchDog(true,WatchDogTimeFeed_);
		}

#endif //defined(_TPE3000_)	
	}
}

void CWatchDog_TPE3000::ResetTimerFeedWatchDog(bool bContinue,unsigned short timeVal)
{
	if (bContinue)
	{
		timerFeedWatchDog_.expires_from_now(boost::posix_time::seconds(timeVal));
		timerFeedWatchDog_.async_wait(boost::bind(&CWatchDog_TPE3000::handle_timerFeedWatchDog,this,boost::asio::placeholders::error));
	}
	else
	{
		timerFeedWatchDog_.cancel();
	}
}

}; //namespace LocalDrive 
