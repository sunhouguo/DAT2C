//#include <stdio.h>
//#include <stdarg.h>
#include <fstream>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include "TextLog.h"
#include "../PublicSupport/Dat2cTypeDef.h"

namespace FileSystem {

/*
CTextLog::CTextLog(void)
{
}
*/

CTextLog::CTextLog(std::string filename,std::string OpenMode,int limit)
				:CLog(filename,OpenMode)
{
	
}

CTextLog::CTextLog(std::string filename,std::string OpenMode)
				:CLog(filename,OpenMode)
{
	
}

CTextLog::~CTextLog(void)
{
}

/*
int CTextLog::InitLog(std::string filename,std::string OpenMode)
{
	strLogPath_ = filename;
	strMode_ = OpenMode;

	return 0;
}
*/

std::string CTextLog::getFileType()
{
	return strTextLog;
}

int CTextLog::WriteToFile(const char * buf)
{
	boost::unique_lock<boost::mutex> uLock(mutex_);

	std::ofstream outfs(strLogPath_.c_str(),std::ios_base::out | std::ios_base::app);

	if (outfs.is_open())
	{
		outfs<<buf;
	}

	outfs.close();
	outfs.clear();

	return 0;
}

int CTextLog::WriteToFileWithSynT(const char * buf)
{
	using namespace boost::posix_time;

	boost::unique_lock<boost::mutex> uLock(mutex_);

	std::ofstream outfs(strLogPath_.c_str(),std::ios::out | std::ios::app);

	if (outfs.is_open())
	{
		ptime now = second_clock::local_time();
		outfs<<to_simple_string(now)<<" "<<buf;
	}

	outfs.close();
	outfs.clear();

	return 0;
}

/*
int CTextLog::AddText(const char * fmt,...)
{
	if (strLogPath_.empty())
	{
		return -1;
	}

	char tBuf[BUFSIZE];

	try      
	{
		va_list argptr;          //分析字符串的格式
		va_start(argptr, fmt);
		vsnprintf(tBuf, BUFSIZE, fmt, argptr);
		va_end(argptr);
	}
	catch (...)
	{
		tBuf[0] = 0;
	}

	if (tBuf[0] == 0)
	{
		return -1;
	}

	return WriteToFile(tBuf);
}

int CTextLog::AddTextWithSynT(const char * fmt,...)
{
	if (strLogPath_.empty())
	{
		return -1;
	}

	char tBuf[BUFSIZE];

	try      
	{
		va_list argptr;          //分析字符串的格式
		va_start(argptr, fmt);
		vsnprintf(tBuf, BUFSIZE, fmt, argptr);
		va_end(argptr);
	}
	catch (...)
	{
		tBuf[0] = 0;
	}

	if (tBuf[0] == 0)
	{
		return -1;
	}

	return WriteToFileWithSynT(tBuf);
}
*/

int CTextLog::AddRecord(std::string strval)
{
	if (strLogPath_.empty())
	{
		return -1;
	}

	return WriteToFile(strval.c_str());
}

int CTextLog::AddRecordWithSynT(std::string strval)
{
	using namespace boost::posix_time;

	if (strLogPath_.empty())
	{
		return -1;
	}

	return WriteToFileWithSynT(strval.c_str());
}

/*int setMaxCounter(size_t var)
{
	int Maxcounter = var;
	return -1;
}*/

}; //namespace PublicSupport 

