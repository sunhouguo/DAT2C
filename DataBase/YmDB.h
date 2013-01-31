#pragma once

#include "../PublicSupport/DynamicArrayDataBase.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

class CYmPoint;

class CYmDB
	:public PublicSupport::CDynamicArrayDataBase<CYmPoint>
{
public:
	CYmDB(void);
	~CYmDB(void);

	int InitDataBase(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);
};

};//namespace DataBase

