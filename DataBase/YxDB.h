#pragma once

#include "../PublicSupport/DynamicArrayDataBase.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

class CYxPoint;

class CYxDB
	:public PublicSupport::CDynamicArrayDataBase<CYxPoint>
{
public:
	CYxDB(void);
	~CYxDB(void);

	int InitDataBase(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);

	int InitDataNodes(FileSystem::CMarkup & xml);

private:
	int LoadXmlAttrib(FileSystem::CMarkup & xml);
	void SaveXmlAttrib(FileSystem::CMarkup & xml);

private:
	bool yxPolar_;
};

};//namespace DataBase

