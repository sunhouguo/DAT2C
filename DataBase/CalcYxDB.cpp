#include "CalcYxDB.h"
#include "CalcYxPoint.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

CCalcYxDB::CCalcYxDB(void)
{
}

CCalcYxDB::~CCalcYxDB(void)
{
}

int CCalcYxDB::InitDataBase(FileSystem::CMarkup & xml)
{
	return PublicSupport::CDynamicArrayDataBase<CCalcYxPoint>::InitDataBase(xml);
}

void CCalcYxDB::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	PublicSupport::CDynamicArrayDataBase<CCalcYxPoint>::SaveXmlCfg(xml);
}

};//namespace DataBase

