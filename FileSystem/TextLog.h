#pragma once
#include <boost/thread/mutex.hpp>
#include "Log.h"

namespace FileSystem {

class CTextLog
	:public CLog
{
public:
	//CTextLog(void);
	CTextLog(std::string filename,std::string OpenMode,int limit);
	CTextLog(std::string filename,std::string OpenMode);
	virtual ~CTextLog(void);
	virtual std::string getFileType();

	//int InitLog(std::string filename,std::string OpenMode);
	//int AddText(const char * fmt,...);
	//int AddTextWithSynT(const char * fmt,...);
	virtual int AddRecord(std::string strval);
	virtual int AddRecordWithSynT(std::string strval);
//	int setMaxCounter(size_t var);
	//std::string getLogPath();

private:
	int WriteToFile(const char * buf);
	int WriteToFileWithSynT(const char * buf);

private:
	boost::mutex mutex_;
};

}; //namespace PublicSupport 
