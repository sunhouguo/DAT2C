#include "YxDB.h"
#include "YxPoint.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

CYxDB::CYxDB(void)
{
	yxPolar_ = CYxPoint::DefaultYxPolar;
}

CYxDB::~CYxDB(void)
{
}

int CYxDB::InitDataBase(FileSystem::CMarkup & xml)
{
	LoadXmlAttrib(xml);

	return PublicSupport::CDynamicArrayDataBase<CYxPoint>::InitDataBase(xml);
}

void CYxDB::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	SaveXmlAttrib(xml);

	PublicSupport::CDynamicArrayDataBase<CYxPoint>::SaveXmlCfg(xml);
}

int CYxDB::InitDataNodes(FileSystem::CMarkup & xml)
{
	for (size_t i=0;i<getDataBaseSum();i++)
	{
		if (yxPolar_ != CYxPoint::DefaultYxPolar)
		{
			getPointDataPtr(i)->setYxPolar(yxPolar_);
		}
	}

	return 0;
}

int CYxDB::LoadXmlAttrib(FileSystem::CMarkup & xml)
{
	std::string strTmp = xml.GetAttrib(strYxPolar);
	boost::algorithm::trim(strTmp);
	if (boost::algorithm::iequals(strboolFalse,strTmp))
	{
		yxPolar_ = false;
	}
	else
	{
		yxPolar_ = true;
	}

	return 0;
}

void CYxDB::SaveXmlAttrib(FileSystem::CMarkup & xml)
{
	if (yxPolar_ != CYxPoint::DefaultYxPolar)
	{
		if (yxPolar_)
		{
			xml.AddAttrib(strYxPolar,strboolTrue);
		}
		else
		{
			xml.AddAttrib(strYxPolar,strboolFalse);
		}
	}
}

};//namespace DataBase

