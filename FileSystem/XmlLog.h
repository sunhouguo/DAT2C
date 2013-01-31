#pragma once
#include <string>
#include <boost/thread/mutex.hpp>
#include "Markup.h"
#include "Log.h"

namespace FileSystem {

class CXmlLog
	:public CLog
{
public:
	CXmlLog(std::string filename,std::string OpenMode,int limit);
	CXmlLog(std::string filename,std::string OpenMode);
	virtual ~CXmlLog(void);
	virtual std::string getFileType();

	virtual int AddRecord(std::string strval);
	virtual int AddRecordWithSynT(std::string strval);

	int getMaxCounter();
	
protected:
	int setMaxCounter(int val);
	int RecordData(std::string var,bool strSynT,CMarkup & xml);
	int ResetRecordCounter(CMarkup & xml);
	int CheckXmlLogCounter(CMarkup & xml);
	int CheckXmlLogExsit(std::string filename,CMarkup & xml);
	int CheckXmlFirstLoad(std::string filename,CMarkup & xml);
	int CreateEmptyXmlLog(std::string filename,CMarkup & xml);

private:
	bool WriteToDB(std::string filename,CMarkup & xml);

protected:
	CMarkup xml_;

private:
	enum
	{
		Default_Maxcounter = 1024  //默认的限制值
	};  

	int iMaxCounter_;
	boost::mutex mutex_;
};

}; //namespace PublicSupport 
