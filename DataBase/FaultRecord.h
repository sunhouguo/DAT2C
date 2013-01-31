#pragma once
#include "DataRecord.h"

namespace DataBase {

class CFaultPoint;

class CFaultRecord :
	public CDataRecord
{
public:
	CFaultRecord(std::string id,int limit);
	CFaultRecord(std::string id);
	virtual ~CFaultRecord(void);

	int AddDataRecord(const CFaultPoint & fault);
};

}; //namespace DataBase
