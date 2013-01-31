#include "FaultPoint.h"

namespace DataBase {

CFaultPoint::CFaultPoint(void)
{
}

CFaultPoint::~CFaultPoint(void)
{
}

unsigned short CFaultPoint::getFaultNO() const
{
	return FaultNO_;
}

int CFaultPoint::setFaultNO(unsigned short val)
{
	FaultNO_ = val;

	return 0;
}

typeYcval CFaultPoint::getFaultVal() const
{
	return FaultVal_;
}

int CFaultPoint::setFaultVal(typeYcval val)
{
	FaultVal_ = val;

	return 0;
}

boost::posix_time::ptime CFaultPoint::getFaultTime() const
{
	return FaultTime_;
}

int CFaultPoint::setFaultTime(boost::posix_time::ptime val)
{
	FaultTime_ = val;
	return 0;
}
}; //namespace DataBase
