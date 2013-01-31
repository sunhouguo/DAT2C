#include <boost/bind.hpp>
#include "DistributedDA.h"

namespace DistributedFA {

CDistributedDA::CDistributedDA(boost::asio::io_service & io_service,std::string id,DataBase::CDAOperate & da_op)
	:self_(da_op)
{
}

CDistributedDA::~CDistributedDA(void)
{
}

int CDistributedDA::ConnectSubAlgorithmSig(CmdRecallSignalType & sig)
{
	SubAlgorithmSigConnection_ = sig.connect(boost::bind(&CDistributedDA::ProcessAlgorithmSig,this,_1,_2,_3,_4));

	return 0;
}

int CDistributedDA::DisconnectSubAlgorithmSig()
{
	SubAlgorithmSigConnection_.disconnect();

	return 0;
}

void CDistributedDA::ProcessAlgorithmSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{

}

}; //namespace DistributedFA
