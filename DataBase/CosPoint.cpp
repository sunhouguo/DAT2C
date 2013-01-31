#include "CosPoint.h"
#include "YxPoint.h"

namespace DataBase {

CCosPoint::CCosPoint(void)
{
	setYxIndex(0);
	setYxVal(0);
	setYxType(single_yx_point);
	setYxQuality(0);
	
}

CCosPoint::CCosPoint(size_t yxIndex,
					 typeYxval yxval,
					 typeYxtype yxType,
					 typeYxQuality yxQuality)
{
	setYxIndex(yxIndex);
	setYxVal(yxval);
	setYxType(yxType);
	setYxQuality(yxQuality);
}

CCosPoint::~CCosPoint(void)
{
}

size_t CCosPoint::getYxIndex() const
{
	return yxIndex_;
}

int CCosPoint::setYxIndex(size_t val)
{
	yxIndex_ = val;

	return 0;
}

typeYxval CCosPoint::getYxVal() const
{
	return yxVal_ & 0x0f;
}

int CCosPoint::setYxVal(typeYxval val)
{
	yxVal_ = (yxVal_ & 0xf0) | (val & 0x0f);

	return 0;
}

typeYxQuality CCosPoint::getYxQuality() const
{
	return yxVal_ & 0xf0;
}

int CCosPoint::setYxQuality(typeYxQuality val)
{
	yxVal_ = (yxVal_ & 0x0f) | (val & 0xf0);

	return 0;
}

typeYxtype CCosPoint::getYxType() const
{
	return yxType_;
}

int CCosPoint::setYxType(typeYxtype val)
{
	if (val == double_yx_point)
	{
		yxType_ = double_yx_point;
	}
	else
	{
		yxType_ = single_yx_point;
	}

	return 0;
}

}; //namespace DataBase 


