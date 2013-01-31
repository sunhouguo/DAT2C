#include "LineDB.h"
#include "Line.h"
#include "../FileSystem/Markup.h"

namespace DataBase {

CLineDB::CLineDB(void)
{
}

CLineDB::~CLineDB(void)
{
}

int CLineDB::InitDataBase(FileSystem::CMarkup & xml)
{
	return PublicSupport::CDynamicArrayDataBase<CLine>::InitDataBase(xml);
}

void CLineDB::SaveXmlCfg(FileSystem::CMarkup & xml)
{
	PublicSupport::CDynamicArrayDataBase<CLine>::SaveXmlCfg(xml);
}

};//namespace DataBase

