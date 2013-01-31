#include "YkDB.h"
#include "YkPoint.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

CYkDB::CYkDB(void)
{
	ykDouble_ = CYkPoint::DefaultYkDouble;
}

CYkDB::~CYkDB(void)
{
}

int CYkDB::InitDataBase(FileSystem::CMarkup & xml)
{
	LoadXmlAttrib(xml);

	return PublicSupport::CDynamicArrayDataBase<CYkPoint>::InitDataBase(xml);
}

void CYkDB::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	SaveXmlAttrib(xml);

	PublicSupport::CDynamicArrayDataBase<CYkPoint>::SaveXmlCfg(xml);
}

int CYkDB::InitDataNodes()
{
	for (size_t i=0;i<getDataBaseSum();i++)
	{
		if (ykDouble_ != CYkPoint::DefaultYkDouble)
		{
			getPointDataPtr(i)->setbHYkDouble(ykDouble_);
		}
	}

	return 0;
}

int CYkDB::LoadXmlAttrib(FileSystem::CMarkup & xml)
{
	std::string strTmp = xml.GetAttrib(strYkDouble);
	boost::algorithm::trim(strTmp);
	if (boost::algorithm::iequals(strboolFalse,strTmp))
	{
		ykDouble_ = false;
	}
	else
	{
		ykDouble_ = true;
	}

	return 0;
}

void CYkDB::SaveXmlAttrib(FileSystem::CMarkup & xml)
{
	if (ykDouble_ != CYkPoint::DefaultYkDouble)
	{
		if (ykDouble_)
		{
			xml.AddAttrib(strYkDouble,strboolTrue);
		}
		else
		{
			xml.AddAttrib(strYkDouble,strboolFalse);
		}
	}
}

};//namespace DataBase

