#pragma once
#include "DataRecord.h"

namespace DataBase {

class CSoePoint;

class CSoeRecord
	:public CDataRecord
{
public:
	CSoeRecord(std::string id,int limit);
	CSoeRecord(std::string id);
	virtual ~CSoeRecord(void);

	int AddDataRecord(const CSoePoint & soe);
};

}; //namespace DataBase

