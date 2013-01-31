#pragma once

#include "../PublicSupport/DynamicArrayDataBase.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

class CCalcYxPoint;

class CCalcYxDB
	:public PublicSupport::CDynamicArrayDataBase<CCalcYxPoint>
{
public:
	CCalcYxDB(void);
	~CCalcYxDB(void);

	int InitDataBase(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);
};

};//namespace DataBase

