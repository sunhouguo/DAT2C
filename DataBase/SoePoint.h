#pragma once
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "CosPoint.h"

namespace DataBase {

class CSoePoint
	:public CCosPoint
{
public:
	CSoePoint(void);
	CSoePoint(size_t yxIndex,typeYxval yxval,typeYxtype yxType,typeYxQuality yxQuality,boost::posix_time::ptime time);
	virtual ~CSoePoint(void);

	boost::posix_time::ptime getYxTime() const;
	int setYxTime(boost::posix_time::ptime val);

protected:
	boost::posix_time::ptime yxTime_;
};

}; //namespace DataBase 
