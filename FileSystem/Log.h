#pragma once
#include <string>

namespace FileSystem {

const std::string strNoLog = "NoLog";
const std::string strTextLog  = "TextLog";
const std::string strXmlLog   = "XmlLog";
const std::string strBoostLog = "BoostLog";

class CLog
{
public:
	CLog(std::string filename,std::string OpenMode);
	virtual ~CLog(void);
	virtual int AddRecord(std::string strval) = 0;
	virtual int AddRecordWithSynT(std::string strval) = 0;
	virtual std::string getFileType();

	std::string getLogPath();

private:
	int GreateFilePath(std::string strFilePath);
		
protected:
	std::string strLogPath_;
	std::string strMode_;
};

}; //namespace FileSystem 
