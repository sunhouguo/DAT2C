#pragma once
#include <boost/asio/io_service.hpp>

namespace CommInterface {

class CCommInterface;
class CServerInterface;
class CCommPara;

//typedef boost::shared_ptr<CommInterface::CCommInterface> share_comm_ptr;

class CCommFactory
{
public:
	~CCommFactory(void);
	static CCommInterface * CreatCommInterface(CCommPara & para,boost::asio::io_service & io_service);
	static CServerInterface * CreateServerInterface(CCommPara & para,boost::asio::io_service & io_service);
	static bool CompareCommPara(CCommPara & srcPara,CCommPara & dstPara,bool & bServer);

private:
	CCommFactory(void);
	static std::string TransCommChannelTypeToString(unsigned char val);
	static int TransCommChannelTypeFromString(std::string val);
};

};//namespace CommInterface 
