#pragma once
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"

namespace DataBase {

using namespace boost::posix_time;

class CFaultPoint
{
public:
	CFaultPoint(void);
	~CFaultPoint(void);

	unsigned short getFaultNO() const;
	int setFaultNO(unsigned short val);
	typeYcval getFaultVal() const;
	int setFaultVal(typeYcval val);
	boost::posix_time::ptime getFaultTime() const;
	int setFaultTime(boost::posix_time::ptime val);

private:
	unsigned short FaultNO_;
	typeYcval FaultVal_;
	boost::posix_time::ptime FaultTime_;
	
};

};//namespace DataBase
