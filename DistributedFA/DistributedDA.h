#pragma once
//#include <boost/thread/mutex.hpp>
//#include <boost/thread/condition.hpp>
#include <boost/asio.hpp>
#include "SelfNode.h"
#include "../PublicSupport/Dat2cTypeDef.h"

namespace DataBase
{
	class CDAOperate;
}

namespace DistributedFA {

class CDistributedDA
{
public:
	CDistributedDA(boost::asio::io_service & io_service,std::string id,DataBase::CDAOperate & da_op);
	~CDistributedDA(void);

	int start(CmdRecallSignalType & sig);
	void stop();

private:
	void ProcessAlgorithmSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val);
	int ConnectSubAlgorithmSig(CmdRecallSignalType & sig);
	int DisconnectSubAlgorithmSig();

private:
	SigConnection SubAlgorithmSigConnection_;
	CSelfNode self_;
};

}; //namespace DistributedFA
