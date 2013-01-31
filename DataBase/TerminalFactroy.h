#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/shared_ptr.hpp>

namespace DataBase {

class CTerminal;
class CSubStation;

typedef boost::shared_ptr<DataBase::CTerminal> share_terminal_ptr;

class CTerminalFactroy
{
public:
	~CTerminalFactroy(void);
	static share_terminal_ptr CreateTerminal(std::string terminalType,boost::asio::io_service & io_service,CSubStation & sub);
	static std::string TransTerminalTypeToString(unsigned short val);

private:
	CTerminalFactroy(void);
	static short TransTerminalTypeFromString( std::string val );
};

}; //namespace DataBase
