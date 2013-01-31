#pragma once
#include <string>
#include "dio.h"

namespace DataBase
{
	class CSubStation;
}

namespace LocalDrive {

class CDIO_Virtual :
	public CDIO
{
public:
	CDIO_Virtual(DataBase::CSubStation & sub);
	virtual ~CDIO_Virtual(void);

	int open();
	void close();

	int check_di(int index);
	int read_di(int index);
	int check_do(int index);
	unsigned char read_do(int index);
	int write_do(int index,bool bCloseOrOpen);

private:
	int CheckAllIndex();
	int ActiveBattery(bool val);
	int ResetDev(bool val);
	int ResetYkAllow(bool val);
	int ResetSubYkAllow();

	int LoadXmlCfg(std::string filename);
	void SaveXmlCfg(std::string filename);

	bool getbAllowYk();
	int setbAllowYk(bool val);
	int getActiveBatteryIndex();
	int setActiveBattryIndex(int val);
	int getResetDevIndex();
	int setResetDevIndex(int val);
	int getResetYkAllowIndex();
	int setResetYkAllowIndex(int val);
	int getAllowYkNotifyIndex();
	int setAllowYkNotifyIndex(int val);

private:
	enum
	{
		DefaultActiveBatteryIndex = 0,
		DefaultResetDevIndex = 1,
		DefaultResetYkAllowIndex  = 2,
		DefaultAllowYkNotifyIndex = 0
	};

	DataBase::CSubStation & sub_;
	bool bAllowYk_;
	int MaxDoNum_;
	int MaxDiNum_;

	int ActiveBatteryIndex_;
	int ResetDevIndex_;
	int ResetYkAllowIndex_;
	int AllowYkNotifyIndex_;
};

}; //namespace LocalDrive

