#pragma once
#include "../PublicSupport/Dat2cTypeDef.h"

namespace DataBase {

class CCosPoint
{
public:
	CCosPoint(void);
	CCosPoint(size_t yxIndex,typeYxval yxval,typeYxtype yxType,typeYxQuality yxQuality);
	virtual ~CCosPoint(void);

	size_t getYxIndex() const;
	int setYxIndex(size_t val);
	typeYxval getYxVal() const;
	int setYxVal(typeYxval val);
	typeYxQuality getYxQuality() const;
	int setYxQuality(typeYxQuality val);
	typeYxtype getYxType() const;
	int setYxType(typeYxtype val);

protected:
	size_t yxIndex_;
	typeYxval yxVal_;
	typeYxtype yxType_;
};

}; //namespace DataBase
