#pragma once
#include "DataRecord.h"
#include "../PublicSupport/DynamicArrayDataBase.h"

namespace DataBase {

class CYcPoint;
//class CSubStation;

//typedef PublicSupport::CDynamicArrayDataBase<CYcPoint> ycDB;

class CYcHisRecord :
	public CDataRecord
{
public:
	CYcHisRecord(std::string id,int limit);
	CYcHisRecord(std::string id);
	virtual ~CYcHisRecord(void);

	int AddDataRecord(PublicSupport::CDynamicArrayDataBase<CYcPoint> & ycDB);
	//int AddDataRecord(CSubStation & sub);
};

};//namespace DataBase

