#include <boost/algorithm/string/predicate.hpp>
#include "DioFactory.h"
#include "DIO_PCM82X.h"
#include "DIO_Virtual.h"

namespace LocalDrive {

#define strDIONull "DIONull"
#define strDIOPCM82X "DIOPCM82X"
#define strDIOVirtual "DIOVirtual"

const unsigned char dio_null = 0;
const unsigned char dio_pcm82x = 1;
const unsigned char dio_virtual = 2;

CDIOFactory::CDIOFactory(void)
{
}


CDIOFactory::~CDIOFactory(void)
{
}

unsigned char CDIOFactory::TransDIOTypeFromString(std::string val)
{
	unsigned char ret = dio_null;

	if (boost::iequals(strDIOPCM82X,val))
	{
		ret = dio_pcm82x;
	}

	if (boost::iequals(strDIOVirtual,val))
	{
		ret = dio_virtual;
	}

	return ret;
}

std::string CDIOFactory::TransDIOTypeToString(unsigned char val)
{
	std::string ret = strDIONull;

	switch(val)
	{
	case dio_pcm82x:
		ret = strDIOPCM82X;
		break;

	case dio_virtual:
		ret = strDIOVirtual;
		break;

	default:
		break;
	}

	return ret;
}

CDIO * CDIOFactory::CreateDIO(std::string dioType,DataBase::CSubStation & sub)
{
	unsigned char ret = TransDIOTypeFromString(dioType);

	CDIO * retPtr = NULL;

	switch(ret)
	{
	case dio_pcm82x:
		retPtr = new CDIO_PCM82X();
		break;

	case dio_virtual:
		retPtr = new CDIO_Virtual(sub);
		break;

	default:
		retPtr = NULL;
		break;
	}

	return retPtr;
}

};//namespace LocalDrive 

