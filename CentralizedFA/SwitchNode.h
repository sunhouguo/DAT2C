#pragma once
#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "OutLetSwitch.h"

namespace FileSystem
{
	class CMarkup;
}

namespace CentralizedFA {

class COutLetSwitch;

class CSwitchNode
	:public COutLetSwitch
{
public:
	CSwitchNode(DataBase::CDAOperate & da_op);
	~CSwitchNode(void);

	bool getbPowerNode();
	unsigned char getSwitchType();

	share_outletswitch_ptr getOutLetSwitch();

	bool CheckNodebLinked(typeID OtherNodeID);

	bool getSwitchProtected();

	int LoadXmlCfg(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);

private:
	unsigned char TransSwitchTypeFromString(std::string val);
	std::string TransSwitchTypeToString(unsigned char val);
	int setSwitchType(unsigned char val);

	int setYcIndex(size_t val);
	size_t getYcIndex();

private:
	enum
	{
		yc_limit_val = 3000,//YcIndex的遥测值小于本值视为失压
	};

	bool bYcOverLoad_;
	size_t UIndex_;

	share_outletswitch_ptr outlet_;

	unsigned char SwitchType_;
	std::vector<typeID> LinkedNodeIDSet_;
};

};//namespace FeederAutomation
