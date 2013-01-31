#pragma once
#include "DIO.h"

namespace LocalDrive {

class CDIO_PCM82X
	:public CDIO
{
public:
	CDIO_PCM82X(void);
	virtual ~CDIO_PCM82X(void);

	int open();
	void close();

	int check_di(int index);
	int read_di(int index);
	int check_do(int index);
	unsigned char read_do(int index);
	int write_do(int index,bool bCloseOrOpen);
	
private:
	int OpenDIO();
	void CloseDIO();

private:
	enum
	{
		Max_DI_Num = 8,
		Max_DO_Num = 4
	};
	int DIOHandle_;
};

} //namespace LocalPCM82X 

