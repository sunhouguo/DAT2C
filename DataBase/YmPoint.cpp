#include "YmPoint.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

CYmPoint::CYmPoint(void)
{
	ymVal_ = DefaultYmVal;
}

CYmPoint::CYmPoint(typeYmval ymval)
				   :ymVal_(ymval)
{

}

CYmPoint::~CYmPoint(void)
{
}

typeYmval CYmPoint::getOriYmVal()
{
	return ymVal_;
}

int CYmPoint::setOriYmVal(typeYmval val)
{
	ymVal_ = val;

	return 0;
}

typeYmquality CYmPoint::getYmQuality()
{
	return ymQuality_;
}

int CYmPoint::setYmQuality(typeYmquality val)
{
	ymQuality_ = val;

	return 0;
}

//xml api
void CYmPoint::SaveXmlCfg(FileSystem::CMarkup & xml)
{
}

int CYmPoint::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	return 0;
}

}; //namespace DataBase 
