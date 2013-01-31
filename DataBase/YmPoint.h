#pragma once
#include "../PublicSupport/Dat2cTypeDef.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

class CYmPoint
{
public:
	CYmPoint(void);
	CYmPoint(typeYmval ymval);
	~CYmPoint(void);

	typeYmval getOriYmVal();
	int setOriYmVal(typeYmval val);
	typeYmquality getYmQuality();
	int setYmQuality(typeYmquality val);

	//xml api
	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml);

protected:
	enum
	{
		DefaultYmVal = 0
	};

	typeYmval ymVal_;
	typeYmquality ymQuality_;
};

}; //namespace DataBase 
