#pragma once
#include <boost/scoped_array.hpp>

namespace PublicSupport {

class CLoopBuf
{
public:
	CLoopBuf(size_t max_buff);
	~CLoopBuf(void);

public:
	int putBuf(unsigned char * src,int count);   //data in, move ptr
	int getBuf(unsigned char * dst,int count);   //data out, move ptr
	int copyBuf(unsigned char * dst,int count);  //data out and do not move ptr;
	int popChar(unsigned char & popedChar);		 //a byte data out, move ptr
	int charNum();								 //num of data left
	int getLoadPtr();                            //for debug
	int getSavePtr();                            //for debug

private:
	//enum
	//{
	//	buffer_length = 4096
	//};
	int savePtr_;
	int loadPtr_;
	//unsigned char buf[buffer_length];
	size_t buffer_length_;
	boost::scoped_array<unsigned char> buf_;
};

}; //namespace PublicSupport
