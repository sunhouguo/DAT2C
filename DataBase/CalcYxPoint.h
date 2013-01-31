#pragma once
#include "YxPoint.h"
#include "../PublicSupport/CalcPoint.h"

namespace DataBase {

class CCalcYxPoint :
	public CYxPoint,
	public PublicSupport::CCalcPoint<typeYxval>
{
public:
	CCalcYxPoint();
	virtual ~CCalcYxPoint(void);

	typeYxval getOriYxVal();
	typeYxval getFinalYxVal();
	typeYxval getLastYxVal();
	int setOriYxVal(typeYxval val);

	void SaveXmlCfg(FileSystem::CMarkup & xml);
	int LoadXmlCfg(FileSystem::CMarkup & xml);

	bool getEnableEffectEvent();

private:
	typeYxval lastVal_;
	bool bEnableEffectEvent_;

};

}; //namespace DataBase

