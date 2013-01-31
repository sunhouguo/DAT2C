#pragma once
#include "../PublicSupport/Dat2cTypeDef.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

#define strYxPolar "YxPolar"

//yx 类型定义
const unsigned char single_yx_point = 0;
const unsigned char double_yx_point = 1;

class CYxPoint
{
public:
	CYxPoint(void);
	CYxPoint(typeYxval yxval,typeYxtype yxtype,bool yxpolar);
	virtual ~CYxPoint(void);

	typeYxval getOriYxVal();
	int setOriYxVal(typeYxval val);
	typeYxtype getYxType();
	int setYxType(typeYxtype val);
	bool getYxPolar();
	int setYxPolar(bool val);
	typeYxQuality getYxQuality();
	int setYxQuality(typeYxQuality val);
	typeYxval getFinalYxVal();

	//xml api
	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml);
	std::string TransYxTypeToString(typeYxtype val);
	typeYxtype TransYxTypeFromString(std::string val);
	std::string TransYxPolarToString(bool val);
	bool TransYxPolarFromString(std::string val);

	//effect event
	int AddEffectYxIndex(size_t indexVal);
	int getEffectYxSum();
	int getEffectYxIndex(int index);

public:
	enum
	{
		DefaultYxPolar = true,
	};

	static unsigned char QualityNegative;
	static unsigned char QualityActive;

protected:
	enum
	{
		DefaultYxVal = 0,
		DefaultYxType = single_yx_point,
	};

	typeYxval yxVal_;
	typeYxtype yxType_;
	bool yxPolar_;
	std::vector<size_t> EffectYxList_;
};

}; //namespace DataBase 
