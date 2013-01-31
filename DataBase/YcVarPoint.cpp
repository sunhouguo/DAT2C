#include "YcVarPoint.h"

namespace DataBase {

CYcVarPoint::CYcVarPoint()
{

}

CYcVarPoint::CYcVarPoint(size_t ycindex,
						 typeYcval ycval,
						 typeYcquality ycQuality,
						 ptime time)
						 :ycIndex_(ycindex),
						 ycVal_(ycval),
						 ycQuality_(ycQuality),
						 ycTime_(time)
{

}

CYcVarPoint::~CYcVarPoint(void)
{
}

size_t CYcVarPoint::getYcIndex()
{
	return ycIndex_;
}

int CYcVarPoint::setYcIndex(size_t val)
{
	ycIndex_ = val;

	return 0;
}

typeYcval CYcVarPoint::getYcVal()
{
	return ycVal_;
}

int CYcVarPoint::setYcVal(typeYcval val)
{
	ycVal_ = val;

	return 0;
}

typeYcquality CYcVarPoint::getYcQuality()
{
	return ycQuality_;
}

int CYcVarPoint::setYcQuality(typeYcquality val)
{
	ycQuality_ = val;

	return 0;
}

ptime CYcVarPoint::getYcTime()
{
	return ycTime_;
}

int CYcVarPoint::setYcTime(ptime val)
{
	ycTime_ = val;

	return 0;
}

}; //namespace DataBase {
