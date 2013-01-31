#include "YcDB.h"
#include "YCPoint.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

CYcDB::CYcDB(void)
{
	YcDead_ = CYcPoint::DefaultYcDead;
	YcPlus_ = CYcPoint::DefaultYcPlus;
	YcMul_ = CYcPoint::DefaultYcMul;
}

CYcDB::~CYcDB(void)
{
}

int CYcDB::InitDataBase(FileSystem::CMarkup & xml)
{
	LoadXmlAttrib(xml);

	return PublicSupport::CDynamicArrayDataBase<CYcPoint>::InitDataBase(xml);
}

void CYcDB::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	SaveXmlAttrib(xml);

	PublicSupport::CDynamicArrayDataBase<CYcPoint>::SaveXmlCfg(xml);
}

int CYcDB::InitDataNodes()
{
	for (size_t i=0;i<getDataBaseSum();i++)
	{
		if (YcDead_ != CYcPoint::DefaultYcDead)
		{
			getPointDataPtr(i)->setYcDeadLimit(YcDead_);
		}

		if (YcPlus_ != CYcPoint::DefaultYcPlus)
		{
			getPointDataPtr(i)->setYcPlus(YcPlus_);
		}

		if (YcMul_ != CYcPoint::DefaultYcMul)
		{
			getPointDataPtr(i)->setYcMul(YcMul_);
		}
	}

	return 0;
}

int CYcDB::LoadXmlAttrib(FileSystem::CMarkup & xml)
{
	std::string strTmp = xml.GetAttrib(strYcDeadLimit);
	boost::algorithm::trim(strTmp);
	try
	{
		YcDead_ = boost::lexical_cast<typeYcdead>(strTmp);
	}
	catch(boost::bad_lexical_cast & e)
	{
		//do nothing,just Ignore sum attrib.
		e.what();
		YcDead_ = CYcPoint::DefaultYcDead;
	}

	strTmp = xml.GetAttrib(strYcPlus);
	boost::algorithm::trim(strTmp);
	try
	{
		YcPlus_ = boost::lexical_cast<typeYcplus>(strTmp);
	}
	catch(boost::bad_lexical_cast & e)
	{
		//do nothing,just Ignore sum attrib.
		e.what();
		YcPlus_ = CYcPoint::DefaultYcPlus;
	}

	strTmp = xml.GetAttrib(strYcMul);
	boost::algorithm::trim(strTmp);
	try
	{
		YcMul_ = boost::lexical_cast<typeYcmul>(strTmp);
	}
	catch(boost::bad_lexical_cast & e)
	{
		//do nothing,just Ignore sum attrib.
		e.what();
		YcMul_ = CYcPoint::DefaultYcMul;
	}

	return 0;
}

void CYcDB::SaveXmlAttrib(FileSystem::CMarkup & xml)
{
	if (YcDead_ != CYcPoint::DefaultYcDead)
	{
		xml.AddAttrib(strYcDeadLimit,YcDead_);
	}

	if (YcPlus_ != CYcPoint::DefaultYcPlus)
	{
		xml.AddAttrib(strYcPlus,YcPlus_);
	}

	if (YcMul_ != CYcPoint::DefaultYcMul)
	{
		xml.AddAttrib(strYcMul,boost::lexical_cast<std::string>(YcMul_));
	}
}

};//namespace DataBase

