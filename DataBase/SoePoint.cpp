#include "SoePoint.h"

namespace DataBase {

using namespace boost::posix_time;

CSoePoint::CSoePoint(void)
					:CCosPoint()
{
}

CSoePoint::CSoePoint(size_t yxIndex,
					 typeYxval yxval,
					 typeYxtype yxType,
					 typeYxQuality yxQuality,
					 ptime time)
					 :yxTime_(time),
					 CCosPoint(yxIndex,yxval,yxType,yxQuality)
					 
{

}

CSoePoint::~CSoePoint(void)
{
}

ptime CSoePoint::getYxTime() const
{
	return yxTime_;
}

int CSoePoint::setYxTime(ptime val)
{
	yxTime_ = val;

	return 0;
}

}; //namespace DataBase 


