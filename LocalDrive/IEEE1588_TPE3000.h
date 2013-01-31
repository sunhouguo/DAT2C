#pragma once
#include <boost/asio/deadline_timer.hpp>

namespace LocalDrive {

class CIEEE1588_TPE3000
{
public:
	CIEEE1588_TPE3000(boost::asio::io_service & io_service,unsigned short timeVal);
	~CIEEE1588_TPE3000(void);
	int run();
	int stop();
	int reset();

private:
	void handle_timerWriteCmosClock(const boost::system::error_code& error,unsigned short timeVal);
	void ResetTimerWriteCmosClock(bool bContinue,unsigned short timeVal);

private:
	unsigned short timeOutWriteCmosClock_;
	boost::asio::deadline_timer timerWriteCmosClock_;
};

};//namespace LocalDrive 

