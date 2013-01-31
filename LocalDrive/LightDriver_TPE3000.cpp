#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"
#include "LightDriver_TPE3000.h"

#if defined(_TPE3000_)
#include "zktylib_led.h"
#endif

namespace LocalDrive {

CLightDriver_TPE3000::CLightDriver_TPE3000(boost::asio::io_service & io_service)
{
	LedHandle_ = -1;

	for(int i=0;i<MAX_LED_NUM;i++)
	{
		led_timers_.push_back(timer_ptr(new boost::asio::deadline_timer(io_service)));
	}
}

CLightDriver_TPE3000::~CLightDriver_TPE3000(void)
{
}

int CLightDriver_TPE3000::run()
{
	return OpenLed();
}

int CLightDriver_TPE3000::stop()
{
	CloseLed();

	return 0;
}

int CLightDriver_TPE3000::reset(int index,std::vector<typeLedStatus> status)
{
	if (LedHandle_ < 0 || index < 0 || index >= getLedSum())
	{
		return -1;
	}

	if (status.empty())
	{
		return -1;
	}

	typeLedStatus s = status.front();
	status.erase(status.begin());

	switch(s)
	{
	case LED_ON:
		WriteLed(index,true);
		break;

	case LED_OFF:
		WriteLed(index,false);
		break;

	case LED_ON_OFF:
		BlingLed(index,1,true);
		break;

	case LED_OFF_ON:
		BlingLed(index,1,false);
		break;

	case LED_BLING:
		BlingLed(index,1);
		break;

	default:
		break;
	}

	if (!status.empty())
	{
		ResetTimerReset(index,status);
	}

	return 0;
}

int CLightDriver_TPE3000::getLedSum()
{
	return led_timers_.size();
}

int CLightDriver_TPE3000::WriteLed(int index,bool OnOrOff)
{
#if defined(_TPE3000_)

	if(LedHandle_ >= 0 && index >= 0 && index < getLedSum())
	{
		unsigned char status;
		zkty_ledread(LedHandle_,0,8,&status);

		if (OnOrOff)
		{
			status |= BYTE_CHECK_TRUE[index];
		}
		else
		{
			status &= BYTE_CHECK_FALSE[index];
		}

		zkty_ledwrite(LedHandle_,0,&status);

		return 0;
	}

#endif //defined(_TPE3000_)	
	
	return -1;
}

int CLightDriver_TPE3000::ReadLed(int index)
{
#if defined(_TPE3000_)

	if(LedHandle_ >= 0 && index >= 0 && index < getLedSum())
	{
		unsigned char status;
		zkty_ledread(LedHandle_,0,1,&status);

		if((BYTE_CHECK_TRUE[index] && status) > 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

#endif //defined(_TPE3000_)	

	return -1;
}

int CLightDriver_TPE3000::BlingLed(int index,int blingTimes,boost::logic::tribool blingstatus/* = boost::logic::indeterminate*/)
{
	if(LedHandle_ < 0 || index < 0 || index >= getLedSum() || blingTimes < 0)
	{
		return -1;
	}

	if (boost::logic::indeterminate(blingstatus))
	{
		int ret = ReadLed(index);
		if (ret == 0)
		{
			WriteLed(index,true);
			WriteLed(index,false);
		}
		else if (ret == 1)
		{
			WriteLed(index,false);
			WriteLed(index,true);
		}
		else
		{
			return -1;
		}
	}
	else if(blingstatus == true)
	{
		WriteLed(index,true);
		WriteLed(index,false);
	}
	else if(blingstatus == false)
	{
		WriteLed(index,false);
		WriteLed(index,true);
	}

	if (--blingTimes > 0)
	{
		ResetTimerBlingLed(index,blingTimes,blingstatus);
	}

	return 0;
}

int CLightDriver_TPE3000::OpenLed()
{
#if defined(_TPE3000_)

	LedHandle_ = zkty_led_open(0);
	if (LedHandle_ < 0)
	{
		unsigned char status  = 0;
		zkty_ledwrite(LedHandle_,0,&status);

		return LedHandle_;
	}

#endif //defined(_TPE3000_)

	return -1;
}

void CLightDriver_TPE3000::CloseLed()
{
#if defined(_TPE3000_)

	if(LedHandle_ >= 0)
	{
		zkty_led_close(LedHandle_);

		LedHandle_ = -1;
	}

#endif //defined(_TPE3000_)	
}

int CLightDriver_TPE3000::ResetTimerBlingLed(int index,int blingTimes,boost::logic::tribool blingstatus)
{
	if (LedHandle_ < 0 || index < 0 || index >= getLedSum())
	{
		return -1;
	}

	led_timers_[index]->expires_from_now(boost::posix_time::milliseconds(BLING_INTERVAL));
	led_timers_[index]->async_wait(boost::bind(&CLightDriver_TPE3000::handle_timerBlingLed,this,boost::asio::placeholders::error,index,blingTimes,blingstatus));

	return 0;
}

void CLightDriver_TPE3000::handle_timerBlingLed(const boost::system::error_code& error,int index,int blingTimes,boost::logic::tribool blingstatus)
{
	if (!error)
	{
		BlingLed(index,blingTimes,blingstatus);
	}
}

int CLightDriver_TPE3000::ResetTimerReset(int index,std::vector<typeLedStatus> status)
{
	if (LedHandle_ < 0 || index < 0 || index >= getLedSum())
	{
		return -1;
	}

	led_timers_[index]->expires_from_now(boost::posix_time::milliseconds(BLING_INTERVAL));
	led_timers_[index]->async_wait(boost::bind(&CLightDriver_TPE3000::handle_timerReset,this,boost::asio::placeholders::error,index,status));

	return 0;
}

void CLightDriver_TPE3000::handle_timerReset(const boost::system::error_code& error,int index,std::vector<typeLedStatus> status)
{
	if (!error)
	{
		reset(index,status);
	}
}

};//namespace LocalDrive

