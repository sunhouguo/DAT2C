#pragma once

namespace LocalDrive {

class CDIO
{
public:
	CDIO(void);
	virtual ~CDIO(void);

	virtual int open() = 0;
	virtual void close() = 0;

	virtual int check_di(int index) = 0;
	virtual int read_di(int index) = 0;
	virtual int check_do(int index) = 0;
	virtual unsigned char read_do(int index) = 0;
	virtual int write_do(int index,bool bCloseOrOpen) = 0;
};

};//namespace LocalDrive 
