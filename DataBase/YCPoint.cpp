#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include "YCPoint.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

unsigned char CYcPoint::QualityNegative = 0x80;
unsigned char CYcPoint::QualityActive = 0x00;

CYcPoint::CYcPoint(void)
{
	ycVal_ = DefaultYcVal;
	ycBackup_ = DefaultYcVal;
	ycPlus_ = DefaultYcPlus;
	ycMul_ = DefaultYcMul;
	ycDeadLimit_ = DefaultYcDead;
	//ycQuality_ = QualityNegative;
	ycQuality_ = QualityActive;

	ycVaildVal_ = ycVal_;

	ResetYcStatVal();
}

CYcPoint::CYcPoint(typeYcval ycval,
		 typeYcval ycBackup,
		 typeYcplus ycplus,
		 typeYcmul ycmul,
		 typeYcdead ycdeadlimit,
		 typeYcquality ycquality)
		 :ycVal_(ycval),
		 ycBackup_(ycBackup),
		 ycPlus_(ycplus),
		 ycMul_(ycmul),
		 ycDeadLimit_(ycdeadlimit),
		 ycQuality_(ycquality)
{
	ycVaildVal_ = ycVal_;

	ResetYcStatVal();
}

CYcPoint::~CYcPoint(void)
{
}

typeYcval CYcPoint::getOriYcVal()
{
	ycBackup_ = ycVal_;

	return ycVal_;
}

int CYcPoint::setOriYcVal(typeYcval val)
{
	ycVal_ = val;

	if (ycVal_ > ycMaxVal_)
	{
		ycMaxVal_ = ycVal_;
	}

	if (ycVal_ < ycMinVal_)
	{
		ycMinVal_ = ycVal_;
	}

	if (ycVal_ <= MinOverLoadYc && ycVal_ >= MaxOverLoadYc)
	{
		ycVaildVal_ = ycVal_;
	}

	return 0;
}

typeYcval CYcPoint::getYcVaild()
{
	return ycVaildVal_;
}

typeYcval CYcPoint::getBackupYcVal()
{
	return ycBackup_;
}

int CYcPoint::setBackupYcVal(typeYcval val)
{
	ycBackup_ = val;

	return 0;
}

typeYcdead CYcPoint::getYcDeadLimit()
{
	return ycDeadLimit_;
}

int CYcPoint::setYcDeadLimit(typeYcdead val)
{
	ycDeadLimit_ = val;
	return 0;
}

typeYcmul CYcPoint::getYcMul()
{
	return ycMul_;
}

int CYcPoint::setYcMul(typeYcmul val)
{
	ycMul_ = val;
	return 0;
}

typeYcplus CYcPoint::getYcPlus()
{
	return ycPlus_;
}

int CYcPoint::setYcPlus(typeYcplus val)
{
	ycPlus_ = val;
	return 0;
}

typeYcquality CYcPoint::getYcQuality()
{
	return ycQuality_;
}

int CYcPoint::setYcQuality(typeYcquality val)
{
	ycQuality_ = val;
	return 0;
}

typeFinalYcval CYcPoint::getFinalYcVal(bool bUseMul /* = false */)
{
	if (bUseMul)
	{
		return (getOriYcVal() + getYcPlus()) * getYcMul();
	}
	else
	{
		return getOriYcVal() + getYcPlus();
	}
	
}

typeYcval CYcPoint::getYcMinVal()
{
	return ycMinVal_;
}

typeYcval CYcPoint::getYcMaxVal()
{
	return ycMaxVal_;
}

void CYcPoint::ResetYcStatVal()
{
	ycMaxVal_ = MinYcVal;
	ycMinVal_ = MaxYcVal;
}

//xml api
void CYcPoint::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	xml.AddElem(strYcPlus,getYcPlus());
	xml.AddElem(strYcMul,getYcPlus());
	xml.AddElem(strYcDeadLimit,getYcDeadLimit());
}

int CYcPoint::LoadXmlCfg(FileSystem::CMarkup & xml)
{
	xml.ResetMainPos();
	if (xml.FindElem(strYcPlus))
	{
		typeYcplus iplus = DefaultYcPlus;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			iplus = boost::lexical_cast<typeYcplus>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<< "非法的遥测增益参数: " << e.what() <<std::endl;
		}
		setYcPlus(iplus);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcMul))
	{
		typeYcmul imul = DefaultYcMul;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			imul = boost::lexical_cast<typeYcmul>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<< "非法的遥测系数参数: " << e.what() <<std::endl;
		}
		setYcMul(imul);
	}

	xml.ResetMainPos();
	if (xml.FindElem(strYcDeadLimit))
	{
		typeYcdead idead = DefaultYcDead;
		try
		{
			std::string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			idead = boost::lexical_cast<typeYcdead>(strTmp);
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<< "非法的遥测死区值参数: " << e.what() <<std::endl;
		}
		setYcDeadLimit(idead);
	}

	return 0;
}

}; //namespace DataBase


