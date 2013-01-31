#include "YmDB.h"
#include "YmPoint.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

CYmDB::CYmDB(void)
{
}

CYmDB::~CYmDB(void)
{
}

int CYmDB::InitDataBase(FileSystem::CMarkup & xml)
{
	return PublicSupport::CDynamicArrayDataBase<CYmPoint>::InitDataBase(xml);
}

void CYmDB::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	PublicSupport::CDynamicArrayDataBase<CYmPoint>::SaveXmlCfg(xml);
}

};//namespace DataBase

