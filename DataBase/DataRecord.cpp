#include "DataRecord.h"

namespace DataBase {

CDataRecord::CDataRecord(std::string id,int limit)
						:record_(id,limit)
{
}

CDataRecord::CDataRecord(std::string id)
						:record_(id,default_limit)
{
}

CDataRecord::~CDataRecord(void)
{
}

int CDataRecord::AddDataRecord(FileSystem::name_val_set & val)
{
	return record_.AddRecordWithSynT(val);
}

int CDataRecord::AddDataRecord(FileSystem::name_val_set & attrib,FileSystem::name_val_set & val)
{
	return record_.AddRecordWithSynT(attrib,val);
}

std::string CDataRecord::getID()
{
	return record_.getLogPath();
}

int CDataRecord::getLimit()
{
	return record_.getMaxCounter();
}

}; //namespace DataBase

