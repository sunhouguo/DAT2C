#pragma once
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase
{
	class CDAOperate;
}

namespace CentralizedFA {

typedef int typeID;

struct stIn
{
	size_t index_;
	int out_;
	bool polar_;

	stIn(size_t val)
	{
		index_ = val;
		out_ = -1;
		polar_ = true;
	}

	stIn(size_t val,bool bVal)
	{
		index_ = val;
		out_ = -1;
		polar_ = bVal;
	}

	stIn(size_t val,int out,bool bVal)
	{
		index_ = val;
		out_ = out;
		polar_ = bVal;
	}
};

struct stOut
{
	size_t index_;
	bool polar_;

	stOut(size_t val)
	{
		index_ = val;
		polar_ = true;
	}

	stOut(size_t val,bool bVal)
	{
		index_ = val;
		polar_ = bVal;
	}
};

class COutLetSwitch
{
public:
	COutLetSwitch(DataBase::CDAOperate & da_op);
	~COutLetSwitch(void);

	int LoadXmlCfg(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);

	std::string getSwitchName();
	typeID getSwitchID();

	bool getSwitchPosition();
	bool getSwitchProtected();
	bool AllowYk();
	int setAllowYk(bool val);
	bool AllowYx();
	
	int OpenSwitch();
	int CloseSwitch();

	void ProcessSubAlgorithmSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);

	bool CheckInputYxSet();
	int WriteOutputYx(int outputindex,typeYxval val,bool bSingleType  = true);
	int WriteInputYx(stIn st,typeYxval val ,bool bSingleType = true);
	//bool ResetOutputYx(typeYxval val,bool bSingleType);

	//void GetLoad();
	//int GetSwitchLoad();
	//int GetYkIndex();

	void SwitchActOver();
	void SwitchActError();

protected:
	int setSwitchID(typeID val);
	int setSwitchName(std::string val);

	int getPositionIndex();
	int setPositionIndex(int val);
	int getProtectIndex();
	int setProtectIndex(int val);
	int getOutYkIndex();
	int setOutYkIndex(int val);

private:
	int ConnectYkSig();
	int DisconnectSubYkSig();

	int OpearteSwitchSel(bool bCloseOrOpen);
	int OpearteSwitchExe(bool bCloseOrOpen);

	void SwitchActStart();
	void SwitchRefuseAct();
	
protected:
	DataBase::CDAOperate & da_op_;

private:
	enum
	{
		wait_ykcon_timeout = 15, //等待遥控响应的超时事件 单位：秒
		//MaxOverLoadYc = 5000, //过负荷遥测值上限
		//MinOverLoadYc = 100   //过负荷遥测值下限
	};
	
	SigConnection SubYkSigConnection_;

	bool bAllowYk_;
	bool bAllowYx_;
	size_t PositionYxIndex_;
	size_t ProtectedYxIndex_;
	size_t OutYkIndex_;
	std::string name_;
	typeID SwitchID_;

	//size_t SwitchLoad_;
	//size_t SwitchLoadValue_;

	boost::mutex ykMutex_;
	boost::condition_variable ykCond_;

	std::vector<stIn> input_set_;
	std::vector<stOut> output_set_;
};

typedef boost::shared_ptr<COutLetSwitch> share_outletswitch_ptr;

}; //namespace FeederAutomation
