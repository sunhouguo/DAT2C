#include <boost/algorithm/string/predicate.hpp>
#include "TerminalFactroy.h"
#include "BF533Terminal.h"
#include "SubStation.h"

namespace DataBase {

const std::string strTerminal = "normal";
const std::string strBF533Terminal = "bf533";

CTerminalFactroy::CTerminalFactroy(void)
{
}

CTerminalFactroy::~CTerminalFactroy(void)
{
}

std::string CTerminalFactroy::TransTerminalTypeToString(unsigned short val)
{
	std::string strTmp = strTerminal;

	switch(val)
	{
	case NormalTerminal:
		strTmp = strTerminal;
		break;

	case BF533Terminal:
		strTmp = strBF533Terminal;
		break;

	default:
		strTmp = strTerminal;
		break;
	}

	return strTerminal;
}

short CTerminalFactroy::TransTerminalTypeFromString( std::string val )
{
	short ret = NormalTerminal;

	if (boost::iequals(strTerminal,val))
	{
		ret = NormalTerminal;
	}
	else if (boost::iequals(strBF533Terminal,val))
	{
		ret = BF533Terminal;
	}
	else
	{
		ret = NormalTerminal;
	}

	return ret;
}

share_terminal_ptr CTerminalFactroy::CreateTerminal(std::string terminalType,boost::asio::io_service & io_service,CSubStation & sub)
{
	int ret = TransTerminalTypeFromString(terminalType);

	switch(ret)
	{
	case NormalTerminal:
		return share_terminal_ptr(new CTerminal(io_service,sub));
		break;

	case BF533Terminal:
		{
			share_bf533_ptr bf533 = share_bf533_ptr(CBF533Terminal::getMyInstance(io_service,sub));
			sub.setBf533Terminal(bf533);
			return bf533;
		}
		break;

	default:
		return share_terminal_ptr(new CTerminal(io_service,sub));
		break;
	}
}

}; //namespace DataBase
