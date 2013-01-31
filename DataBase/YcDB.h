#pragma once

#include "../PublicSupport/DynamicArrayDataBase.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

class CYcPoint;

class CYcDB
	:public PublicSupport::CDynamicArrayDataBase<CYcPoint>
{
public:
	CYcDB(void);
	~CYcDB(void);

	int InitDataBase(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);

private:
	int LoadXmlAttrib(FileSystem::CMarkup & xml);
	void SaveXmlAttrib(FileSystem::CMarkup & xml);

	int InitDataNodes();

private:
	typeYcdead YcDead_;
	typeYcplus YcPlus_;
	typeYcmul YcMul_;
};

};//namespace DataBase

