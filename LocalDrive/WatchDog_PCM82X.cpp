#include <iostream>
#include <boost/bind.hpp>
#include "WatchDog_PCM82X.h"

#if defined(_PCM82X_)
#include "zktywatchdog.h"
#endif //#if defined(_PCM82X_)

namespace LocalDrive {

CWatchDog_PCM82X::CWatchDog_PCM82X(boost::asio::io_service & io_service,bool bEnableLog)
					:CWatchDog(bEnableLog),
					io_service_(io_service),
					timerFeedWatchDog_(io_service)
{
	feedCount_ = 0;
	WatchDogHandle_ = -1;
	WatchDogTimeOut_ = DefaultWatchDogTimeOut;
	WatchDogTimeFeed_ = DefaultWatchDogTimeFeed;
}

CWatchDog_PCM82X::~CWatchDog_PCM82X(void)
{
}

/*
int CWatchDog_PCM82X::AddLogWithSynT(std::string strVal)
{
	if (Log_)
	{
		return Log_->AddRecordWithSynT(strVal);
	}

	return -1;
}
*/

int CWatchDog_PCM82X::run()
{
	return OpenWatchDog();
}

int CWatchDog_PCM82X::stop()
{
	CloseWatchDog();
	
	return 0;
}

int CWatchDog_PCM82X::reset()
{
	ResetTimerFeedWatchDog(true,0);

	return 0;
}

int CWatchDog_PCM82X::OpenWatchDog()
{
#if defined(_PCM82X_)

	WatchDogHandle_ = open("/dev/watchdog",O_RDWR | O_NONBLOCK);
	if(WatchDogHandle_ < 0)
	{
		std::ostringstream ostr;
		ostr<<"PCM82X watchdog device open fail"<<std::endl;
		AddLogWithSynT(ostr.str());
		std::cerr<<ostr.str();

		return WatchDogHandle_;
	}

	ioctl(WatchDogHandle_,WDIOC_SETTIMEOUT,&WatchDogTimeOut_);
	ResetTimerFeedWatchDog(true,WatchDogTimeFeed_);

	std::ostringstream ostr;
	ostr<<"PCM82X start dog"<<std::endl;
	AddLogWithSynT(ostr.str());

	return 0;

#endif //defined(_PCM82X_)

	return -1;
}

void CWatchDog_PCM82X::CloseWatchDog()
{
	ResetTimerFeedWatchDog(false,WatchDogTimeFeed_);

#if defined(_PCM82X_)

	if(WatchDogHandle_ >= 0)
	{
		write(WatchDogHandle_,"V",1);
		close(WatchDogHandle_);
		WatchDogHandle_ = -1;

		std::ostringstream ostr;
		ostr<<"PCM82X close dog"<<std::endl;
		AddLogWithSynT(ostr.str());
	}

#endif //defined(_PCM82X_)	

}

void CWatchDog_PCM82X::handle_timerFeedWatchDog(const boost::system::error_code& error)
{
	if(!error)
	{
#if defined(_PCM82X_)

		if (WatchDogHandle_ >= 0)
		{
			std::ostringstream ostr;
			ostr<<"PCM82X feed dog on the "<<feedCount_++<<" times."<<std::endl;
			AddLogWithSynT(ostr.str());

			write(WatchDogHandle_,"K",1);

			ResetTimerFeedWatchDog(true,WatchDogTimeFeed_);
		}

#endif //defined(_PCM82X_)	
	}
}

void CWatchDog_PCM82X::ResetTimerFeedWatchDog(bool bContinue,unsigned short timeVal)
{
	if (bContinue)
	{
		timerFeedWatchDog_.expires_from_now(boost::posix_time::seconds(timeVal));
		timerFeedWatchDog_.async_wait(boost::bind(&CWatchDog_PCM82X::handle_timerFeedWatchDog,this,boost::asio::placeholders::error));
	}
	else
	{
		timerFeedWatchDog_.cancel();
	}
}

}//namespace LocalDrive

