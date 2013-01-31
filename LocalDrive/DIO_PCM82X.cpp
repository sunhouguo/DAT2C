#include <iostream>
#include "DIO_PCM82X.h"
#include "../PublicSupport/Dat2cTypeDef.h"

#if defined(_PCM82X_)
#include "zktylib.h"
#endif //#if defined(_PCM82X_)

namespace LocalDrive {

CDIO_PCM82X::CDIO_PCM82X(void)
{
}

CDIO_PCM82X::~CDIO_PCM82X(void)
{
}

int CDIO_PCM82X::open()
{
	return OpenDIO();
}

void CDIO_PCM82X::close()
{
	CloseDIO();
}

int CDIO_PCM82X::OpenDIO()
{
#if defined(_PCM82X_)

	DIOHandle_ = zkty_deviceopen(0);
	if (DIOHandle_ < 0)
	{
		std::cerr<<"dio device open fail"<<std::endl;
		return DIOHandle_;
	}

	return 0;

#endif //defined(_PCM82X_)

	return -1;
}

void CDIO_PCM82X::CloseDIO()
{
#if defined(_PCM82X_)

	if(DIOHandle_ >= 0)
	{
		zkty_deviceclose(DIOHandle_);
		DIOHandle_ = -1;
	}

#endif //defined(_PCM82X_)	
}

int CDIO_PCM82X::read_di(int index)
{
#if defined(_PCM82X_)

	if ((index < 0) || (index >= (int)Max_DI_Num) || (DIOHandle_ < 0))
	{	  
		return -1;
	}

	unsigned char byteVal;
	int ret = zkty_diread(DIOHandle_,0,1,&byteVal);
	if (ret < 0)
	{
		return ret;
	}

	if((BYTE_CHECK_TRUE[index] & byteVal) > 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}

#endif //defined(_PCM82X_)

	return -1;
}

int CDIO_PCM82X::check_di(int index)
{
#if defined(_PCM82X_)

	if ((index < 0) || (index >= (int)Max_DI_Num) || (DIOHandle_ < 0))
	{
		return -1;
	}

	return 0;

#endif //defined(_PCM82X_)

	return -1;
}

int CDIO_PCM82X::check_do(int index)
{
#if defined(_PCM82X_)

	if ((index < 0) || (index >= (int)Max_DO_Num) || (DIOHandle_ < 0))
	{
		return -1;
	}

	return 0;

#endif //defined(_PCM82X_)

	return -1;
}

unsigned char CDIO_PCM82X::read_do(int index)
{
#if defined(_PCM82X_)

	if ((index < 0) || (index >= (int)Max_DO_Num) || (DIOHandle_ < 0))
	{
		return -1;
	}

	unsigned char byteVal;
	zkty_doread(DIOHandle_,0,1,&byteVal);

	unsigned char ret;
	if((BYTE_CHECK_TRUE[index] & byteVal) > 0)
	{
		ret = 1;
	}
	else
	{
		ret = 0;
	}

	return ret;

#endif //defined(_PCM82X_)

	return -1;
}

int CDIO_PCM82X::write_do(int index,bool bCloseOrOpen)
{
#if defined(_PCM82X_)

	if ((index < 0) || (index >= (int)Max_DO_Num) || (DIOHandle_ < 0))
	{
		return -1;
	}

	unsigned char byteVal;
	int ret = zkty_doread(DIOHandle_,0,1,&byteVal);
	if (ret < 0)
	{
		return ret;
	}

	if (bCloseOrOpen)
	{
		byteVal = BYTE_CHECK_TRUE[index] | byteVal;
	}
	else
	{
		byteVal = BYTE_CHECK_FALSE[index] & byteVal;
	}

	ret = zkty_dowrite(DIOHandle_,0,1,&byteVal);
	
	return ret;

#endif //defined(_PCM82X_)

	return -1;
}

} //namespace LocalPCM82X
