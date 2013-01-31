#pragma once

#include "../PublicSupport/DynamicArrayDataBase.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

class CLine;

class CLineDB
	:public PublicSupport::CDynamicArrayDataBase<CLine>
{
public:
	CLineDB(void);
	~CLineDB(void);

	int InitDataBase(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);
};

};//namespace DataBase

