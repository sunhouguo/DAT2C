#include <iostream>
#include <boost/bind.hpp>
#include "WatchDog_BF518.h"

#if defined(_BF518_)
#include "zktywatchdog.h"
#endif

namespace LocalDrive {

CWatchDog_BF518::CWatchDog_BF518(boost::asio::io_service & io_service,bool bEnableLog)
					:CWatchDog(bEnableLog),
					io_service_(io_service),
					timerFeedWatchDog_(io_service),timerRunLight_(io_service)
{
	feedCount_ = 0;

	RunLightHandle_ = -1;
	WatchDogHandle_ = -1;

	RunLightFlag_ = false;

	WatchDogTimeOut_ = DefaultWatchDogTimeOut;
	WatchDogTimeFeed_ = DefaultWatchDogTimeFeed;
	RunLightTime_ = DefaultRunSingalTime;
}

CWatchDog_BF518::~CWatchDog_BF518(void)
{
}

int CWatchDog_BF518::run()
{
	OpenRunLight();

	return OpenWatchDog();
}

int CWatchDog_BF518::stop()
{
	CloseWatchDog();
	CloseRunLight();
	
	return 0;
}

int CWatchDog_BF518::reset()
{
	ResetTimerFeedWatchDog(true,0);

	return 0;
}

int CWatchDog_BF518::OpenRunLight()
{
#if defined(_BF518_)

	RunLightHandle_ = open("/dev/BF518RUN",O_WRONLY);

	if(RunLightHandle_ < 0)
	{
		std::ostringstream ostr;
		ostr<<"BF518RUN device open fail"<<std::endl;
		AddLogWithSynT(ostr.str());
		std::cerr<<ostr.str();

		return RunLightHandle_;
	}

	//std::cout<<"Start the BF518RUN Light sucess!"<<std::endl;
	ResetTimerRunLight(true,RunLightTime_);

	return 0;

#endif //defined(_BF518_)

	return -1;
}

int CWatchDog_BF518::OpenWatchDog()
{
#if defined(_BF518_)

	WatchDogHandle_ = open("/dev/watchdog",O_WRONLY/* O_RDWR | O_NONBLOCK*/);
	if(WatchDogHandle_ < 0)
	{
		std::ostringstream ostr;
		ostr<<"BF518 watchdog device open fail"<<std::endl;
		AddLogWithSynT(ostr.str());
		std::cerr<<ostr.str();

		return WatchDogHandle_;
	}

	ioctl(WatchDogHandle_,WDIOC_SETTIMEOUT,&WatchDogTimeOut_); 

	ResetTimerFeedWatchDog(true,WatchDogTimeFeed_);

	std::ostringstream ostr;
	ostr<<"BF518 start dog"<<std::endl;
	AddLogWithSynT(ostr.str());

	return 0;

#endif //defined(_BF518_)

	return -1;
}

void CWatchDog_BF518::CloseWatchDog()
{
	ResetTimerFeedWatchDog(false,WatchDogTimeFeed_);

#if defined(_BF518_)

	if(WatchDogHandle_ >= 0)
	{
		write(WatchDogHandle_,"V",1);
		close(WatchDogHandle_);
		WatchDogHandle_ = -1;

		std::ostringstream ostr;
		ostr<<"BF518 close dog"<<std::endl;
		AddLogWithSynT(ostr.str());
	}

#endif //defined(_BF518_)	

}

void CWatchDog_BF518::CloseRunLight()
{
	ResetTimerRunLight(false,RunLightTime_);

#if defined(_BF518_)

	if(RunLightHandle_ >= 0)
	{
		ioctl(RunLightHandle_,0, 1);
		close(RunLightHandle_);
		RunLightHandle_ = -1;

		std::ostringstream ostr;
		ostr<<"BF518 close runlight"<<std::endl;
		AddLogWithSynT(ostr.str());
	}

#endif //defined(_BF518_)	
}

void CWatchDog_BF518::handle_timerFeedWatchDog(const boost::system::error_code& error)
{
	if(!error)
	{
#if defined(_BF518_)

		if (WatchDogHandle_ >= 0)
		{
			std::ostringstream ostr;
			ostr<<"BF518 feed dog on the "<<feedCount_++<<" times."<<std::endl;
			AddLogWithSynT(ostr.str());

			write(WatchDogHandle_,"\0",1);

			ResetTimerFeedWatchDog(true,WatchDogTimeFeed_);
		}

#endif //defined(_BF518_)	
	}
}

void CWatchDog_BF518::handle_timerRunLight(const boost::system::error_code& error)
{
	if(!error)
	{

#if defined(_BF518_)

		if (RunLightHandle_ >= 0)
		{
			RunLightFlag_ = !RunLightFlag_;

			if (RunLightFlag_)
			{
				ioctl(RunLightHandle_,1, 1);
			}
			else
			{
				ioctl(RunLightHandle_,0, 1);
			}
			

			ResetTimerRunLight(true,RunLightTime_);
		}

#endif //defined(_BF518_)	
	}

}

void CWatchDog_BF518::ResetTimerFeedWatchDog(bool bContinue,unsigned short timeVal)
{
	if (bContinue)
	{
		timerFeedWatchDog_.expires_from_now(boost::posix_time::seconds(timeVal));
		timerFeedWatchDog_.async_wait(boost::bind(&CWatchDog_BF518::handle_timerFeedWatchDog,this,boost::asio::placeholders::error));
	}
	else
	{
		timerFeedWatchDog_.cancel();
	}
}

void CWatchDog_BF518::ResetTimerRunLight(bool bContinue,unsigned short timeVal)
{
	if (bContinue)
	{
		timerRunLight_.expires_from_now(boost::posix_time::seconds(timeVal));
		timerRunLight_.async_wait(boost::bind(&CWatchDog_BF518::handle_timerRunLight,this,boost::asio::placeholders::error));
	}
	else
	{
		timerRunLight_.cancel();
	}
}

}//namespace LocalDrive

