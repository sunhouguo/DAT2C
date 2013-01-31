#pragma once
#include "DataRecord.h"

namespace DataBase {

class CCosPoint;

class CCosRecord :
	public CDataRecord
{
public:
	CCosRecord(std::string id,int limit);
	CCosRecord(std::string id);
	virtual ~CCosRecord(void);

	int AddDataRecord(const CCosPoint & cos);
};

}; //namespace DataBase
