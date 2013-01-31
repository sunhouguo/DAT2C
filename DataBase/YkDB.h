#pragma once

#include "../PublicSupport/DynamicArrayDataBase.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

class CYkPoint;

class CYkDB
	:public PublicSupport::CDynamicArrayDataBase<CYkPoint>
{
public:
	CYkDB(void);
	~CYkDB(void);

	int InitDataBase(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);

	int InitDataNodes();

private:
	int LoadXmlAttrib(FileSystem::CMarkup & xml);
	void SaveXmlAttrib(FileSystem::CMarkup & xml);

private:
	bool ykDouble_;
};

};//namespace DataBase

