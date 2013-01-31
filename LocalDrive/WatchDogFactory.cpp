#include <boost/algorithm/string/predicate.hpp>
#include "WatchDogFactory.h"
#include "WatchDog_PCM82X.h"
#include "WatchDog_TPE3000.h"
#include "WatchDog_BF518.h"

namespace LocalDrive {

#define strWatchDogNull "WatchDogNull"
#define strWatchDogPCM82X "WatchDogPCM82X"
#define strWatchDogTPE3000 "WatchDogTPE3000"
#define strWatchDogBF518 "WatchDogBF518"

const unsigned char watchdog_null = 0;
const unsigned char watchdog_pcm82x = 1;
const unsigned char watchdog_tpe3000 = 2;
const unsigned char watchdog_bf518 = 3;

CWatchDogFactory::CWatchDogFactory(void)
{
}


CWatchDogFactory::~CWatchDogFactory(void)
{
}

CWatchDog * CWatchDogFactory::CreateWatchDog(std::string watchdogType,boost::asio::io_service & io_service,bool bEnableLog)
{
	unsigned char ret = TransWatchDogTypeFromString(watchdogType);

	CWatchDog * retPtr = NULL;

	switch(ret)
	{
	case watchdog_pcm82x:
		retPtr = new CWatchDog_PCM82X(io_service,bEnableLog);
		break;

	case watchdog_tpe3000:
		retPtr = new CWatchDog_TPE3000(io_service,bEnableLog);
		break;

	case watchdog_bf518:
		retPtr = new CWatchDog_BF518(io_service,bEnableLog);
		break;

	default:
		retPtr = new CWatchDog_PCM82X(io_service,bEnableLog);
		break;
	}

	return retPtr;
}

std::string CWatchDogFactory::TransWatchDogTypeToString(unsigned short val)
{
	std::string ret = strWatchDogNull;

	switch(val)
	{
	case watchdog_pcm82x:
		ret = strWatchDogPCM82X;
		break;

	case watchdog_tpe3000:
		ret = strWatchDogTPE3000;
		break;

	case watchdog_bf518:
		ret = strWatchDogBF518;
		break;

	default:
		break;
	}

	return ret;
}

unsigned char CWatchDogFactory::TransWatchDogTypeFromString( std::string val )
{
	unsigned char ret = watchdog_null;

	if (boost::iequals(strWatchDogPCM82X,val))
	{
		ret = watchdog_pcm82x;
	}
	else if(boost::iequals(strWatchDogTPE3000,val))
	{
		ret = watchdog_tpe3000;
	}
	else if(boost::iequals(strWatchDogBF518,val))
	{
		ret = watchdog_bf518;
	}

	return ret;
}

}; //namespace LocalDrive

