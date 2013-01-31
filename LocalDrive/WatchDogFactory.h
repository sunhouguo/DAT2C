#pragma once
#include <boost/asio/io_service.hpp>

namespace LocalDrive {

class CWatchDog;

class CWatchDogFactory
{
public:
	~CWatchDogFactory(void);
	static CWatchDog * CreateWatchDog(std::string watchdogType,boost::asio::io_service & io_service,bool bEnableLog);
	static std::string TransWatchDogTypeToString(unsigned short val);

private:
	CWatchDogFactory(void);
	static unsigned char TransWatchDogTypeFromString( std::string val );
};

};//namespace LocalDrive

