#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include "LogFactory.h"
#include "TextLog.h"
#include "XmlLog.h"
//#include "BoostLog.h"

namespace FileSystem {

const unsigned char NoLog = 0;
const unsigned char TextLog = 1;
const unsigned char XmlLog = 2;
//const unsigned char BoostLog = 3;

CLogFactory::CLogFactory(void)
{
}


CLogFactory::~CLogFactory(void)
{
}

CLog * CLogFactory::CreateLog(std::string log_id,std::string log_type,std::string log_limit /*="0"*/)
{
	unsigned char ret = TransLogTypeFromString(log_type);

	int iLimit = -1;
	if (!log_limit.empty())
	{
		try
		{
			iLimit = boost::lexical_cast<int>(log_limit);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::cerr<<"CLogFactory::CreateLog "<<e.what()<<std::endl;
		} 
	}
	else
	{
		iLimit = 0;
	}

	CLog * retPtr = NULL;

	switch(ret)
	{
	case TextLog:
		if (iLimit > 0)
		{
			retPtr = new CTextLog(log_id,"a",iLimit);
		}
		else
		{
			retPtr = new CTextLog(log_id,"a");
		}
		break;

	case XmlLog:
		if (iLimit > 0)
		{
			retPtr = new CXmlLog(log_id,"",iLimit);
		}
		else
		{
			retPtr = new CXmlLog(log_id,"");
		}
		break;

//	case BoostLog:
//		if (iLimit > 0)
//		{
//			retPtr = new CBoostLog(log_id,"",iLimit);
//		}
//		else
//		{
//			retPtr = new CBoostLog(log_id,"");
//		}
//		break;

	default:
		retPtr = NULL;
		break;
	}

	return retPtr;
}

std::string CLogFactory::TransLogTypeToString(unsigned short val)
{
	std::string ret = strNoLog;

	switch(val)
	{
	case TextLog:
		ret = strTextLog;
		break;

	case XmlLog:
		ret = strXmlLog;
		break;

//	case BoostLog:
//		ret = strBoostLog;
//		break;

	default:
		ret = strNoLog;
		break;
	}

	return ret;
}

unsigned char CLogFactory::TransLogTypeFromString( std::string val )
{
	unsigned char ret = NoLog;

	if (boost::iequals(strTextLog,val))
	{
		ret = TextLog;
	}
	else if(boost::iequals(strXmlLog,val))
	{
		ret = XmlLog;
	}
//	else if (boost::iequals(strBoostLog,val))
//	{
//		ret = BoostLog;
//	}

	return ret;
}

}; //namespace FileSystem

