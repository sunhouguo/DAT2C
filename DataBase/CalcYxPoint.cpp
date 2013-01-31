#include "CalcYxPoint.h"

namespace DataBase {

#define strEnableEvent "EnableEvent"

CCalcYxPoint::CCalcYxPoint()
{
	bEnableEffectEvent_ = false;
}

CCalcYxPoint::~CCalcYxPoint(void)
{
}

typeYxval CCalcYxPoint::getOriYxVal()
{
	lastVal_ = getPointVal();
	return lastVal_;
}

typeYxval CCalcYxPoint::getFinalYxVal()
{
	if (getYxPolar())
	{
		return getOriYxVal();
	}
	else
	{
		if (getYxType() == single_yx_point)
		{
			return (getOriYxVal() & 0xfe) | ((~getOriYxVal()) & 0x01);
		}
		else if (getYxType() == double_yx_point)
		{
			return (getOriYxVal() & 0xfc) | ((~getOriYxVal()) & 0x03);
		}
		else
		{
			return getOriYxVal();
		}
	}
}

typeYxval CCalcYxPoint::getLastYxVal()
{
	return lastVal_;
}

int CCalcYxPoint::setOriYxVal(typeYxval val)
{
	return -1;
}

void CCalcYxPoint::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	if (bEnableEffectEvent_)
	{
		xml.AddElem(strEnableEvent,strboolTrue);
	}
	else
	{
		xml.AddElem(strEnableEvent,strboolFalse);
	}

	CYxPoint::SaveXmlCfg(xml);

	PublicSupport::CCalcPoint<typeYxval>::SaveXmlCfg(xml);
}

int CCalcYxPoint::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	xml.ResetMainPos();
	if (xml.FindElem(strEnableEvent))
	{
		bEnableEffectEvent_ = false;

		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		if (boost::algorithm::iequals(strboolTrue,strTmp))
		{
			bEnableEffectEvent_ = true;
		}
	}

	CYxPoint::LoadXmlCfg(xml);

	PublicSupport::CCalcPoint<typeYxval>::LoadXmlCfg(xml);

	return 0;
}

bool CCalcYxPoint::getEnableEffectEvent()
{
	return bEnableEffectEvent_;
}

}; //namespace DataBase

