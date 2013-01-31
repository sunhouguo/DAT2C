#pragma once
#include "../PublicSupport/Dat2cTypeDef.h"

namespace FileSystem
{
	class CMarkup;
}

namespace DataBase {

#define strYcDeadLimit "YcDeadLimit"
#define strYcMul "YcMul"
#define strYcPlus "YcPlus"

class CYcPoint
{
public:
	CYcPoint(void);
	CYcPoint(typeYcval ycval,typeYcval ycBackup,typeYcplus ycplus,typeYcmul ycmul,typeYcdead ycdeadlimit,typeYcquality ycquality);
	~CYcPoint(void);

	typeYcval getOriYcVal();
	int setOriYcVal(typeYcval val);
	typeYcval getBackupYcVal();
	int setBackupYcVal(typeYcval val);
	typeYcplus getYcPlus();
	int setYcPlus(typeYcplus val);
	typeYcmul getYcMul();
	int setYcMul(typeYcmul val);
	typeYcdead getYcDeadLimit();
	int setYcDeadLimit(typeYcdead val);
	typeYcquality getYcQuality();
	int setYcQuality(typeYcquality val);
	typeFinalYcval getFinalYcVal(bool bUseMul = false);

	//xml api
	int LoadXmlCfg(FileSystem::CMarkup & xml);
	void SaveXmlCfg(FileSystem::CMarkup & xml);
	
	void ResetYcStatVal();
	typeYcval getYcMinVal();
	typeYcval getYcMaxVal();

	typeYcval getYcVaild();

public:
	enum
	{
		DefaultYcPlus = 0,

#ifdef _BF518_
		DefaultYcDead = 30,
#else
		DefaultYcDead = 2,
#endif

		DefaultYcMul = 1,
	};

	static unsigned char QualityNegative;
	static unsigned char QualityActive;

protected:
	enum
	{
		DefaultYcVal = 0,
		DefaultYcQuality = 0,

		MaxOverLoadYc = 5000, //过负荷遥测值上限
		MinOverLoadYc = 100   //过负荷遥测值下限
	};

	typeYcval ycVal_;
	typeYcval ycBackup_;
	typeYcplus ycPlus_;
	typeYcmul ycMul_;
	typeYcdead ycDeadLimit_;
	typeYcquality ycQuality_;

	typeYcval ycMinVal_;
	typeYcval ycMaxVal_;

	typeYcval ycVaildVal_;

};

}; //namespace DataBase 
