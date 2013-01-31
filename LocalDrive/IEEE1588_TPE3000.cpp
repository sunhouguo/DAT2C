#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include "IEEE1588_TPE3000.h"

namespace LocalDrive {

CIEEE1588_TPE3000::CIEEE1588_TPE3000(boost::asio::io_service & io_service,unsigned short timeVal)
	:timerWriteCmosClock_(io_service),
	timeOutWriteCmosClock_(timeVal)
{
}


CIEEE1588_TPE3000::~CIEEE1588_TPE3000(void)
{
}

int CIEEE1588_TPE3000::run()
{
	ResetTimerWriteCmosClock(true,timeOutWriteCmosClock_);

	return 0;
}

int CIEEE1588_TPE3000::stop()
{
	ResetTimerWriteCmosClock(false,0);

	return 0;
}

int CIEEE1588_TPE3000::reset()
{
	return 0;
}

void CIEEE1588_TPE3000::handle_timerWriteCmosClock(const boost::system::error_code& error,unsigned short timeVal)
{
	if(!error)
	{
#if defined(_TPE3000_)

		system("hwclock -w");//ÉèÖÃÓ²¼þÊ±ÖÓ

		ResetTimerWriteCmosClock(true,timeVal);

#endif //defined(_TPE3000_)	
	}
}

void CIEEE1588_TPE3000::ResetTimerWriteCmosClock(bool bContinue,unsigned short timeVal)
{
	if (bContinue && timeVal > 0)
	{
		timerWriteCmosClock_.expires_from_now(boost::posix_time::minutes(timeVal));
		timerWriteCmosClock_.async_wait(boost::bind(&CIEEE1588_TPE3000::handle_timerWriteCmosClock,this,boost::asio::placeholders::error,timeVal));
	}
	else
	{
		timerWriteCmosClock_.cancel();
	}
}

};//namespace LocalDrive
