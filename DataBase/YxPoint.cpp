#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "YxPoint.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

#define strYxType "YxType"
#define strDoubleYxVal "DoulbePointYx"
#define strSingleYxVal "SinglePointYx"

unsigned char CYxPoint::QualityNegative = 0x80;
unsigned char CYxPoint::QualityActive = 0x00;

CYxPoint::CYxPoint(void)
{
	//yxVal_ = DefaultYxVal | QualityNegative;
	yxVal_ = DefaultYxVal;
	yxType_ = DefaultYxType;
	yxPolar_ = DefaultYxPolar;
}

CYxPoint::CYxPoint(typeYxval yxval,
				   typeYxtype yxtype,
				   bool yxpolar)
				   :yxVal_(yxval),
				   yxType_(yxtype),
				   yxPolar_(yxpolar)
{

}

CYxPoint::~CYxPoint(void)
{
}

typeYxval CYxPoint::getOriYxVal()
{
	return yxVal_ & 0x0f;
}

int CYxPoint::setOriYxVal(typeYxval val)
{
	yxVal_ = (yxVal_ & 0xf0) | (val & 0x0f);

	return 0;
}

bool CYxPoint::getYxPolar()
{
	return yxPolar_;
}

int CYxPoint::setYxPolar(bool val)
{
	yxPolar_ = val;

	return 0;
}

typeYxtype CYxPoint::getYxType()
{
	return yxType_;
}

int CYxPoint::setYxType(typeYxtype val)
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

typeYxQuality CYxPoint::getYxQuality()
{
	return yxVal_ & 0xf0;
}

int CYxPoint::setYxQuality(typeYxQuality val)
{
	yxVal_ = (yxVal_ & 0x0f) | (val & 0xf0);
	
	return 0;
}

typeYxval CYxPoint::getFinalYxVal()
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

//xml api
void CYxPoint::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddElem(strYxType,TransYxTypeToString(getYxType()));
	xml.AddElem(strYxPolar,TransYxPolarToString(getYxPolar()));
}

int CYxPoint::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	///*
	xml.ResetMainPos();
	if (xml.FindElem(strYxType))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		typeYxtype val = TransYxTypeFromString(strTmp);
		setYxType(val);
	}
	//*/

	xml.ResetMainPos();
	if (xml.FindElem(strYxPolar))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		bool val = TransYxPolarFromString(strTmp);
		setYxPolar(val);
	}

	return 0;
}

std::string CYxPoint::TransYxTypeToString(typeYxtype val)
{
	std::string strTmp = strSingleYxVal;

	switch (val)
	{
	case single_yx_point:
		strTmp = strSingleYxVal;
		break;

	case double_yx_point:
		strTmp = strDoubleYxVal;
		break;
	}

	return strTmp;
}

typeYxtype CYxPoint::TransYxTypeFromString(std::string val)
{
	typeYxtype ret = DefaultYxType;

	if (boost::iequals(strSingleYxVal,val))
	{
		ret = single_yx_point;
	}
	else if (boost::iequals(strDoubleYxVal,val))
	{
		ret = double_yx_point;
	}

	return ret;
}

std::string CYxPoint::TransYxPolarToString(bool val)
{
	std::string strTmp = strboolTrue;

	if (val)
	{
		strTmp = strboolTrue;
	}
	else
	{
		strTmp = strboolFalse;
	}

	return strTmp;
}

bool CYxPoint::TransYxPolarFromString(std::string val)
{
	bool ret = DefaultYxPolar;

	if (boost::iequals(strboolTrue,val))
	{
		ret = true;
	}
	else if (boost::iequals(strboolFalse,val))
	{
		ret = false;
	}
	
	return ret;
}

int CYxPoint::AddEffectYxIndex(size_t indexVal)
{
	for (size_t i=0;i<getEffectYxSum();i++)
	{
		if(indexVal == getEffectYxIndex(i))
		{
			return -1;
		}
	}

	EffectYxList_.push_back(indexVal);

	return 0;
}

int CYxPoint::getEffectYxSum()
{
	return EffectYxList_.size();
}

int CYxPoint::getEffectYxIndex(int index)
{
	if (index < 0 || index >= getEffectYxSum())
	{
		return -1;
	}

	return EffectYxList_[index];
}

}; //namespace DataBase 


