#pragma once
#include "../FileSystem/XmlRecord.h"

namespace DataBase {

class CDataRecord
{
public:
	CDataRecord(std::string id,int limit);
	CDataRecord(std::string id);
	virtual ~CDataRecord(void);

	std::string getID();
	int getLimit();

protected:
	int AddDataRecord(FileSystem::name_val_set & val);
	int AddDataRecord(FileSystem::name_val_set & attrib,FileSystem::name_val_set & val);

private:
	enum
	{
		default_limit = 256,
	};

	FileSystem::CXmlRecord record_;
};

}; //namespace DataBase
