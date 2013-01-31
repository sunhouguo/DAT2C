#pragma once
#include "DataRecord.h"
#include "../PublicSupport/DynamicArrayDataBase.h"

namespace DataBase {

class CYcPoint;

class CYcStatRecord
	:public CDataRecord
{
public:
	CYcStatRecord(std::string id,int limit);
	CYcStatRecord(std::string id);
	virtual ~CYcStatRecord(void);

	int AddDataRecord(PublicSupport::CDynamicArrayDataBase<CYcPoint> & ycDB);
};

}; //namespace DataBase
