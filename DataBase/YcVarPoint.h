#pragma once
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"

namespace DataBase {

using namespace boost::posix_time;

class CYcVarPoint
{
public:
	CYcVarPoint();
	//CYcVarPoint(size_t ycindex,typeYcval ycval);
	CYcVarPoint(size_t ycindex,typeYcval ycval,typeYcquality ycQuality,ptime time);
	~CYcVarPoint(void);

	size_t getYcIndex();
	int setYcIndex(size_t val);
	typeYcval getYcVal();
	int setYcVal(typeYcval val);
	typeYcquality getYcQuality();
	int setYcQuality(typeYcquality val);
	ptime getYcTime();
	int setYcTime(ptime val);

protected:
	size_t ycIndex_;
	typeYcval ycVal_;
	typeYcquality ycQuality_;
	ptime ycTime_;
};

}; //namespace DataBase 
