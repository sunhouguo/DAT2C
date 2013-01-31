#include "Bluetooth.h"
//#include "../FileSystem/TextLog.h"
#include "../FileSystem/Log.h"
#include "../FileSystem/LogFactory.h"

namespace LocalDrive {

const std::string filename = "Bluetooth.log";

CBluetooth::CBluetooth(bool bEnableLog)
{
	if (bEnableLog)
	{
		Log_.reset(FileSystem::CLogFactory::CreateLog(filename,FileSystem::strTextLog));
	}
}

CBluetooth::~CBluetooth(void)
{
}

int CBluetooth::AddLogWithSynT(std::string strVal)
{
	if (Log_)
	{
		return Log_->AddRecordWithSynT(strVal);
	}

	return -1;
}

}; //namespace LocalDrive 
