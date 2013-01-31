#pragma once
#include <vector>
#include <boost/asio/io_service.hpp>
#include "../PublicSupport/ZdTimer.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase
{
	class CSubStation;
}


namespace PublicSupport
{
    class CZdTimer;
}

namespace LocalDrive {

class CBatteryActive
{
public:
	CBatteryActive(boost::asio::io_service & io_service,std::string strCfg,DataBase::CSubStation & sub);
	~CBatteryActive(void);

	int run();
	int stop();
	int reset();

public:
	int LoadXml(std::string strCfg);
	void SaveXml(std::string strCfg);

private:
	void handle_timerBatteryActive(const boost::system::error_code& error);
	void ResetTimerBatteryActive(bool bContinue,boost::posix_time::time_duration timeVal);
	boost::posix_time::time_duration getMinTimeFromNow();
	int ActiveBattery(bool val);
	//void setEnableBA(bool val);
	//bool getEnableBA();

private:
	DataBase::CSubStation & sub_;
	//boost::asio::io_service & io_service_;
	//boost::asio::deadline_timer timerBatteryActive_;
	PublicSupport::CZdTimer zdtimer_;
	std::vector<boost::posix_time::ptime> times_;
};

}; //namespace LocalDrive
