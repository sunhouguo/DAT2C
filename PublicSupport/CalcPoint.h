#pragma once

#ifndef CalcPoint_H
#define CalcPoint_H

#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/exception/all.hpp>
#include "../PublicSupport/Dat2cTypeDef.h"
#include "../FileSystem/Markup.h"

const unsigned char AndByBit = 1; //与遥信
const unsigned char OrByBit = 2;  //或遥信
const unsigned char DoubleBit = 3;//双点遥信

namespace PublicSupport {

#define strCalcType "CalcType"
#define strAndCalcTypeVal "AndByBit"
#define strOrCalcTypeVal "OrByBit"
#define strDoubleCalcTypeVal "DoubleByBit"
#define strUnknownCalcTypeVal "OtherCalcType"
#define strLeftIndex "LeftIndex"
#define strRightIndex "RightIndex"

template <class Elemtype> class CCalcPoint
{
public:
	CCalcPoint();
	virtual ~CCalcPoint(void);

	Elemtype getPointVal();

	size_t getLeftIndex();
	int setLeftIndex(size_t val);
	size_t getRightIndex();
	int setRightIndex(size_t val);
	unsigned char getCalcType();
	int setCalcType(unsigned char val);

	int setFuctionObj(boost::function<Elemtype(size_t)> val);

	//xml api
	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml);
	std::string TransCalcTypeToString(unsigned char val);
	char TransCalcTypeFromeString(std::string val);

protected:
	boost::function<Elemtype(size_t)> getVal_;
	size_t leftIndex;
	size_t rightIndex;
	unsigned char calcType_;

};

template <class Elemtype> CCalcPoint<Elemtype>::CCalcPoint()
{
}

template <class Elemtype> CCalcPoint<Elemtype>::~CCalcPoint(void)
{
}

template <class Elemtype> Elemtype CCalcPoint<Elemtype>::getPointVal()
{
	Elemtype ret;

	if (getVal_)
	{
		switch (calcType_)
		{
		case AndByBit:
			ret = ((getVal_(leftIndex) & getVal_(rightIndex)) & 0x0f) | (getVal_(leftIndex) & 0xf0) | (getVal_(rightIndex) & 0xf0);
			break;

		case OrByBit:
			ret = getVal_(leftIndex) | getVal_(rightIndex);
			break;

		case DoubleBit:
			ret = (getVal_(leftIndex) & 0x01) | ((getVal_(rightIndex) & 0x01) << 1);
			break;

		default:
			break;
		}
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("CCalcPoint::getVal_ fuction isn't assigned.");
	}

	return ret;
}

template <class Elemtype> size_t CCalcPoint<Elemtype>::getLeftIndex()
{
	return leftIndex;
}

template <class Elemtype> int CCalcPoint<Elemtype>::setLeftIndex(size_t val)
{
	leftIndex = val;

	return 0;
}

template <class Elemtype> size_t CCalcPoint<Elemtype>::getRightIndex()
{
	return rightIndex;
}

template <class Elemtype> int CCalcPoint<Elemtype>::setRightIndex(size_t val)
{
	rightIndex = val;

	return 0;
}

template <class Elemtype> unsigned char CCalcPoint<Elemtype>::getCalcType()
{
	return calcType_;
}

template <class Elemtype> int CCalcPoint<Elemtype>::setCalcType(unsigned char val)
{
	calcType_ = val;

	return 0;
}

template <class Elemtype> int CCalcPoint<Elemtype>::setFuctionObj(boost::function<Elemtype(size_t)> val)
{
	getVal_ = val;

	return 0;
}

template <class Elemtype> void CCalcPoint<Elemtype>::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddElem(strCalcType,TransCalcTypeToString(getCalcType()));
	xml.AddElem(strLeftIndex,getLeftIndex());
	xml.AddElem(strRightIndex,getRightIndex());
}

template <class Elemtype> int CCalcPoint<Elemtype>::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	xml.ResetMainPos();
	if (xml.FindElem(strCalcType))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		char val = TransCalcTypeFromeString(strTmp);
		if (val < 0)
		{
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未定义的计算类型参数");
		}
		else
		{
			setCalcType(val);
		}
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未找到计算类型参数配置项");
	}

	xml.ResetMainPos();
	if (xml.FindElem(strLeftIndex))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			setLeftIndex(boost::lexical_cast<size_t>(strTmp));
		}
		catch(boost::bad_lexical_cast & e)
		{
			std::ostringstream ostr;
			ostr<<"未定义的左点号参数"<<e.what();
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
		
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未找到左点号参数配置项");
	}

	xml.ResetMainPos();
	if (xml.FindElem(strRightIndex))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			setRightIndex(boost::lexical_cast<size_t>(strTmp));
		}
		catch(boost::bad_lexical_cast & e)
		{
			std::ostringstream ostr;
			ostr<<"未定义的右点号参数"<<e.what();
			throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
		}
		
	}
	else
	{
		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name("未找到左点号参数配置项");
	}

	return 0;
}

template <class Elemtype> std::string CCalcPoint<Elemtype>::TransCalcTypeToString(unsigned char val)
{
	std::string strTmp = strUnknownCalcTypeVal;

	switch (val)
	{
	case AndByBit:
		strTmp = strAndCalcTypeVal;
		break;

	case OrByBit:
		strTmp = strOrCalcTypeVal;
		break;

	case DoubleBit:
		strTmp = strDoubleCalcTypeVal;
		break;

	default:
		break;
	}

	return strTmp;
}

template <class Elemtype> char CCalcPoint<Elemtype>::TransCalcTypeFromeString(std::string val)
{
	char ret = -1;

	if (boost::algorithm::iequals(strAndCalcTypeVal,val))
	{
		ret = AndByBit;
	}
	else if (boost::algorithm::iequals(strOrCalcTypeVal,val))
	{
		ret = OrByBit;
	}
	else if (boost::algorithm::iequals(strDoubleCalcTypeVal,val))
	{
		ret = DoubleBit;
	}

	return ret;
}

}; //namespace PublicSupport

#endif

