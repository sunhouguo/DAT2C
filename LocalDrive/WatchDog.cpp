#include "WatchDog.h"
//#include "../FileSystem/TextLog.h"
#include "../FileSystem/Log.h"
#include "../FileSystem/LogFactory.h"

namespace LocalDrive {

const std::string filename = "WatchDog.log";

CWatchDog::CWatchDog(bool bEnableLog)
{
	if (bEnableLog)
	{
		Log_.reset(FileSystem::CLogFactory::CreateLog(filename,FileSystem::strTextLog));
	}
}

CWatchDog::~CWatchDog(void)
{
}

int CWatchDog::AddLogWithSynT(std::string strVal)
{
	if (Log_)
	{
		return Log_->AddRecordWithSynT(strVal);
	}

	return -1;
}

}; //namespace LocalDrive 
