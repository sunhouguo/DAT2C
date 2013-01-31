#pragma once
#include <boost/asio/io_service.hpp>

namespace Protocol {

class CProtocol;

class CProtocolFactory
{
public:
	virtual ~CProtocolFactory(void);
	static CProtocol * CreateProtocol(std::string protocolType,boost::asio::io_service & io_service);

private:
	CProtocolFactory(void);
	static std::string TransProtocolTypeToString(unsigned short val);
	static short TransProtocolTyeFromString( std::string val );
};

}; //namespace Protocol
