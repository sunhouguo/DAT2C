#pragma once
#include "log.h"

namespace FileSystem {

class CBoostLog :
	public CLog
{
public:
	CBoostLog(std::string filename,std::string OpenMode,int limit);
	CBoostLog(std::string filename,std::string OpenMode);
	virtual ~CBoostLog(void);

	virtual std::string getFileType();

	virtual int AddRecord(std::string strval);
	virtual int AddRecordWithSynT(std::string strval);

private:
	int InitSink(std::string log_id);

private:
	// Here we define our application severity levels.
	enum severity_level
	{
		normal,
		notification,
		warning,
		error,
		critical
	};
};

};//namespace FileSystem

